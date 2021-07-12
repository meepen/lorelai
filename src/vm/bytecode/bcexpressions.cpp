#include "expressions.hpp"
#include "generator.hpp"
#include "parser/expressions.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser;
using namespace lorelai::bytecode;

#define GENERATEFUNC(t) static void generate_##t(bytecodegenerator &gen, parser::node *_expr, std::uint32_t target, std::uint32_t size)
#define INIT(t) expressions::t &expr = dynamic_cast<expressions::t &>(*_expr)


GENERATEFUNC(numberexpression) {
	// when size = 0, no need to init
	if (size == 0) {
		return;
	}
	INIT(numberexpression);
	gen.emit(bytecode::instruction_opcode_NUMBER, target, gen.add(expr.data));
}

GENERATEFUNC(stringexpression) {
	// when size = 0, no need to init
	if (size == 0) {
		return;
	}
	INIT(stringexpression);
	gen.emit(bytecode::instruction_opcode_STRING, target, gen.add(expr.data));
}

GENERATEFUNC(enclosedexpression) {
	// when size = 0, no need to init
	if (size == 0) {
		return;
	}
	INIT(enclosedexpression);
	gen.runexpressionhandler(expr.enclosed, target, size > 0 ? 1 : 0);
}

std::unordered_map<string, bytecode::instruction_opcode> binoplookup = {
	{ "%", bytecode::instruction_opcode_MODULO },
	{ "-", bytecode::instruction_opcode_SUBTRACT },
	{ "+", bytecode::instruction_opcode_ADD },
	{ "/", bytecode::instruction_opcode_DIVIDE },
	{ "*", bytecode::instruction_opcode_MULTIPLY },
	{ "..", bytecode::instruction_opcode_CONCAT },
	{ "^", bytecode::instruction_opcode_POWER },
	{ "and", bytecode::instruction_opcode_AND },
	{ "&&", bytecode::instruction_opcode_AND },
	{ "or", bytecode::instruction_opcode_OR },
	{ "||", bytecode::instruction_opcode_OR },
	{ "<", bytecode::instruction_opcode_LESSTHAN },
	{ "<=", bytecode::instruction_opcode_LESSTHANEQUAL },
	{ ">", bytecode::instruction_opcode_GREATERTHAN },
	{ ">=", bytecode::instruction_opcode_GREATERTHANEQUAL },
	{ "==", bytecode::instruction_opcode_EQUALS },
	{ "~=", bytecode::instruction_opcode_NOTEQUALS },
	{ "!=", bytecode::instruction_opcode_NOTEQUALS },
};

/*
given

local _0, _1, _2, _3

_0 = _1 - _2 + _3


the ideal output would be

#1 | MOV              | 4, 1, 1
#2 | SUBTRACT         | 4, 4, 2
#3 | ADD              | 4, 4, 3
#4 | MOV              | 0, 4, 1

currently:

#4 | MOV              | 0, 1, 1
#5 | MOV              | 4, 2, 1
#6 | SUBTRACT         | 0, 0, 4
#7 | MOV              | 4, 3, 1
#8 | ADD              | 0, 0, 4

*/


class binopsimplifier {
public:
	binopsimplifier(bytecodegenerator &_gen, expressions::binopexpression &expr, std::uint32_t target, std::uint32_t size) : gen(_gen) {
		if (size == 0) {
			auto find = binoplookup.find(expr.op);
			if (find == binoplookup.end()) {
				throw;
			}
			auto opcode = find->second;
			if (opcode == bytecode::instruction_opcode_AND || opcode == bytecode::instruction_opcode_OR) {
				// these do not have overloads, can ignore binop part
				auto temp = gen.curfunc.getslots(1);
				gen.runexpressionhandler(expr.lhs, temp, 1);
				gen.runexpressionhandler(expr.rhs, temp, 1);
				gen.curfunc.freeslots(temp, 1);
			}
			else {
				auto temp = gen.curfunc.getslots(2);
				gen.runexpressionhandler(expr.lhs, temp, 1);
				gen.runexpressionhandler(expr.rhs, temp + 1, 1);
				gen.emit(opcode, temp, temp, temp + 1);
				gen.curfunc.freeslots(temp, 2);
			}
		}
		else {
			std::uint32_t rhs_stack;
			bool free_right = false;
			gen.runexpressionhandler(expr.lhs, target, 1);

			if (free_right = !get(expr.rhs, &rhs_stack, false)) {
				rhs_stack = gen.curfunc.getslots(1);
				gen.runexpressionhandler(expr.rhs, rhs_stack, 1);
			}

			gen.emit(binoplookup[expr.op], target, target, rhs_stack);

			if (free_right) {
				gen.curfunc.freeslots(rhs_stack, 1);
			}
		}
	}

	bool get(parser::node *expr, std::uint32_t *stackposout, bool leftside) {
		if (auto name = dynamic_cast<expressions::nameexpression *>(expr)) {
			if (gen.curfunc.hasvariable(name->name)) {
				*stackposout = gen.curfunc.varlookup[name->name];
				return true;
			}
		}

		return false;
	}

public:
	bytecodegenerator &gen;
};

GENERATEFUNC(binopexpression) {
	INIT(binopexpression);
	binopsimplifier simplify(gen, expr, target, size);
}

std::unordered_map<string, bytecode::instruction_opcode> unoplookup = {
	{ "not", bytecode::instruction_opcode_NOT },
	{ "!", bytecode::instruction_opcode_NOT },
	{ "#", bytecode::instruction_opcode_LENGTH },
	{ "-", bytecode::instruction_opcode_MINUS },
};

GENERATEFUNC(unopexpression) {
	INIT(unopexpression);

	if (size > 0) {
		gen.runexpressionhandler(expr.expr, target, 1);
		gen.emit(unoplookup[expr.op], target, target);
	}
	else if (unoplookup[expr.op] != bytecode::instruction_opcode_NOT) {
		auto temp = gen.curfunc.getslots(1);

		gen.runexpressionhandler(expr.expr, temp, 1);
		gen.emit(unoplookup[expr.op], temp, temp);

		gen.curfunc.freeslots(temp, 1);
	}
	else {
		gen.runexpressionhandler(expr.expr, target, 0);
	}
}

GENERATEFUNC(nilexpression) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 2);
}
GENERATEFUNC(falseexpression) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 1);
}
GENERATEFUNC(trueexpression) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 0);
}

GENERATEFUNC(indexexpression) {
	INIT(indexexpression);
	
	bool is_temp = size == 0;

	if (is_temp) {
		size = 1;
		target = gen.curfunc.getslots(size);
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.curfunc.getslots(1);
	gen.runexpressionhandler(expr.index, temp, 1);
	gen.emit(bytecode::instruction_opcode_INDEX, target, target, temp);
	gen.curfunc.freeslots(temp, 1);

	if (is_temp) {
		gen.curfunc.freeslots(target, size);
	}
}


GENERATEFUNC(dotexpression) {
	INIT(dotexpression);

	bool is_temp = size == 0;

	if (is_temp) {
		size = 1;
		target = gen.curfunc.getslots(size);
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.curfunc.getslots(1);
	expressions::stringexpression index(expr.index);
	gen.runexpressionhandler(&index, temp, 1);
	gen.emit(bytecode::instruction_opcode_INDEX, target, target, temp);
	gen.curfunc.freeslots(temp, 1);

	if (is_temp) {
		gen.curfunc.freeslots(target, size);
	}
}

GENERATEFUNC(nameexpression) {
	if (size == 0) {
		return;
	}

	INIT(nameexpression);

	if (gen.curfunc.hasvariable(expr.name)) {
		gen.mov(target, gen.curfunc.varlookup[expr.name], 1);
	}
	/*
	else if(gen.curfunc.hasupvalue(expr.name)) {
		// TODO
	} */
	else if (expr.name == "_ENV") {
		gen.emit(bytecode::instruction_opcode_ENVIRONMENT, target);
	}
	else {
		gen.emit(bytecode::instruction_opcode_ENVIRONMENTGET, target, gen.add(expr.name));
	}
}

GENERATEFUNC(functioncallexpression) {
	INIT(functioncallexpression);
	auto &arglist = *dynamic_cast<args *>(expr.arglist);

	std::uint32_t stacksize = std::max(size, static_cast<std::uint32_t>(arglist.arglist.size() + 1 + (expr.methodname ? 1 : 0)));

	bool using_temp = stacksize > size || target == -1;
	auto functionindex = target;
	if (using_temp) {
		functionindex = gen.curfunc.getslots(stacksize);
	}
	auto argsindex = functionindex + 1;

	if (expr.methodname) {
		gen.runexpressionhandler(expr.funcexpr, argsindex, 1);
		expressions::stringexpression method(*expr.methodname);
		gen.runexpressionhandler(&method, functionindex, 1);
		gen.emit(bytecode::instruction_opcode_INDEX, functionindex, argsindex, functionindex);

		argsindex++;
	}
	else {
		gen.runexpressionhandler(expr.funcexpr, functionindex, 1);
	}

	auto opcode = bytecode::instruction_opcode_CALL;

	auto argsize = arglist.arglist.size();

	for (int i = 0; i < arglist.arglist.size(); i++) {
		auto &arg = arglist.arglist[i];	
		if (dynamic_cast<expressions::varargexpression *>(arg) && i == arglist.arglist.size() - 1) {
			opcode = bytecode::instruction_opcode_CALLV;
			argsize--;
		}
		else if (auto call = dynamic_cast<expressions::functioncallexpression *>(arg) && i == arglist.arglist.size() - 1) {
			opcode = bytecode::instruction_opcode_CALLM;
			gen.runexpressionhandler(arg, -1, 0);
			argsize--;
		}
		else {
			gen.runexpressionhandler(arg, argsindex + i, 1);
		}
	}

	gen.emit(opcode, functionindex, argsize, target == -1 ? 0 : size + 1);

	if (using_temp) {
		if (target != -1 && size != 0) {
			gen.mov(target, functionindex, size);
		}
		gen.curfunc.freeslots(functionindex, stacksize);
	}
}

static void populate_tablevalue(bytecode::tablevalue *val, bytecodegenerator &gen, node *_expr) {
	if (auto numvalue = dynamic_cast<expressions::numberexpression *>(_expr)) {
		val->set_type(bytecode::tablevalue_valuetype_NUMBER);
		val->set_index(gen.add(numvalue->data));
	}
	else if (auto strvalue = dynamic_cast<expressions::stringexpression *>(_expr)) {
		val->set_type(bytecode::tablevalue_valuetype_STRING);
		val->set_index(gen.add(strvalue->data));
	}
	else if (auto strvalue = dynamic_cast<expressions::trueexpression *>(_expr)) {
		val->set_type(bytecode::tablevalue_valuetype_CONSTANT);
		val->set_index(0);
	}
	else if (auto strvalue = dynamic_cast<expressions::falseexpression *>(_expr)) {
		val->set_type(bytecode::tablevalue_valuetype_CONSTANT);
		val->set_index(1);
	}
	else if (auto strvalue = dynamic_cast<expressions::nilexpression *>(_expr)) {
		val->set_type(bytecode::tablevalue_valuetype_CONSTANT);
		val->set_index(2);
	}
	else {
		throw exception(string("cannot create tablevalue for ") + _expr->tostring());
	}
}

GENERATEFUNC(tableexpression) {
	INIT(tableexpression);
	auto bcid = gen.curfunc.proto->tables_size();
	gen.emit(bytecode::instruction_opcode_TABLE, target, bcid);
	auto bcobj = gen.curfunc.proto->add_tables();
	for (auto &hashpart : expr.hashpart) {
		auto bckv = bcobj->add_hashpart();
		auto key = new bytecode::tablevalue;
		try {
			populate_tablevalue(key, gen, hashpart.first);
		}
		catch (std::exception &e) {
			delete key;
			throw;
		}

		bckv->set_allocated_key(key);
		auto value = new bytecode::tablevalue;
		try {
			populate_tablevalue(value, gen, hashpart.second);
		}
		catch (std::exception &e) {
			delete value;
			throw;
		}
		bckv->set_allocated_value(value);
	}

	for (auto &arraypart : expr.arraypart) {
		auto bcarraypart = bcobj->add_arraypart();
		populate_tablevalue(bcarraypart, gen, arraypart);
	}
}

GENERATEFUNC(functionexpression) {
	INIT(functionexpression);
	gen.emit(bytecode::instruction_opcode_FNEW, gen.protomap[_expr]);
}


#define ADD(x) { typeid(expressions::x), generate_##x }
std::unordered_map<std::type_index, expressiongenerator> bytecode::expressionmap = {
	ADD(numberexpression),
	ADD(nilexpression),
	ADD(falseexpression),
	ADD(trueexpression),
	ADD(stringexpression),
	ADD(enclosedexpression),
	ADD(binopexpression),
	ADD(unopexpression),
	ADD(functioncallexpression),
	ADD(tableexpression),
	ADD(indexexpression),
	ADD(dotexpression),
	ADD(nameexpression),
	ADD(functioncallexpression),
};
#undef ADD
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
	gen.emit(prototype::OP_NUMBER, target, gen.add(expr.data));
}

GENERATEFUNC(stringexpression) {
	// when size = 0, no need to init
	if (size == 0) {
		return;
	}
	INIT(stringexpression);
	gen.emit(prototype::OP_STRING, target, gen.add(expr.data));
}

GENERATEFUNC(enclosedexpression) {
	// when size = 0, no need to init
	if (size == 0) {
		return;
	}
	INIT(enclosedexpression);
	gen.runexpressionhandler(expr.enclosed, target, 1);
}

std::unordered_map<string, prototype::_opcode> binoplookup = {
	{ "%", prototype::OP_MODULO },
	{ "-", prototype::OP_SUBTRACT },
	{ "+", prototype::OP_ADD },
	{ "/", prototype::OP_DIVIDE },
	{ "*", prototype::OP_MULTIPLY },
	{ "..", prototype::OP_CONCAT },
	{ "^", prototype::OP_POWER },
	{ "and", prototype::OP_AND },
	{ "&&", prototype::OP_AND },
	{ "or", prototype::OP_OR },
	{ "||", prototype::OP_OR },
	{ "<", prototype::OP_LESSTHAN },
	{ "<=", prototype::OP_LESSTHANEQUAL },
	{ ">", prototype::OP_GREATERTHAN },
	{ ">=", prototype::OP_GREATERTHANEQUAL },
	{ "==", prototype::OP_EQUALS },
	{ "~=", prototype::OP_NOTEQUALS },
	{ "!=", prototype::OP_NOTEQUALS },
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
			if (opcode == prototype::OP_AND || opcode == prototype::OP_OR) {
				// these do not have overloads, can ignore binop part
				auto temp = gen.funcptr->getslots(1);
				gen.runexpressionhandler(expr.lhs, temp, 1);
				gen.runexpressionhandler(expr.rhs, temp, 1);
				gen.funcptr->freeslots(temp, 1);
			}
			else {
				auto temp = gen.funcptr->getslots(2);
				gen.runexpressionhandler(expr.lhs, temp, 1);
				gen.runexpressionhandler(expr.rhs, temp + 1, 1);
				gen.emit(opcode, temp, temp, temp + 1);
				gen.funcptr->freeslots(temp, 2);
			}
		}
		else {
			std::uint32_t rhs_stack;
			bool free_right = false;
			gen.runexpressionhandler(expr.lhs, target, 1);

			if ((free_right = !get(expr.rhs, &rhs_stack, false))) {
				rhs_stack = gen.funcptr->getslots(1);
				gen.runexpressionhandler(expr.rhs, rhs_stack, 1);
			}

			gen.emit(binoplookup[expr.op], target, target, rhs_stack);

			if (free_right) {
				gen.funcptr->freeslots(rhs_stack, 1);
			}
		}
	}

	bool get(parser::node *expr, std::uint32_t *stackposout, bool leftside) {
		if (auto name = dynamic_cast<expressions::nameexpression *>(expr)) {
			if (gen.funcptr->hasvariable(name->name)) {
				*stackposout = gen.funcptr->varlookup[name->name];
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

std::unordered_map<string, prototype::_opcode> unoplookup = {
	{ "not", prototype::OP_NOT },
	{ "!", prototype::OP_NOT },
	{ "#", prototype::OP_LENGTH },
	{ "-", prototype::OP_MINUS },
};

GENERATEFUNC(unopexpression) {
	INIT(unopexpression);

	if (size > 0) {
		gen.runexpressionhandler(expr.expr, target, 1);
		gen.emit(unoplookup[expr.op], target, target);
	}
	else if (unoplookup[expr.op] != prototype::OP_NOT) {
		auto temp = gen.funcptr->getslots(1);

		gen.runexpressionhandler(expr.expr, temp, 1);
		gen.emit(unoplookup[expr.op], temp, temp);

		gen.funcptr->freeslots(temp, 1);
	}
	else {
		gen.runexpressionhandler(expr.expr, target, 0);
	}
}

GENERATEFUNC(nilexpression) {
	if (size == 0) {
		return;
	}

	gen.emit(prototype::OP_CONSTANT, target, 2);
}
GENERATEFUNC(falseexpression) {
	if (size == 0) {
		return;
	}

	gen.emit(prototype::OP_CONSTANT, target, 1);
}
GENERATEFUNC(trueexpression) {
	if (size == 0) {
		return;
	}

	gen.emit(prototype::OP_CONSTANT, target, 0);
}

GENERATEFUNC(indexexpression) {
	INIT(indexexpression);
	
	bool is_temp = size == 0;

	if (is_temp) {
		size = 1;
		target = gen.funcptr->getslots(size);
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.funcptr->getslots(1);
	gen.runexpressionhandler(expr.index, temp, 1);
	gen.emit(prototype::OP_INDEX, target, target, temp);
	gen.funcptr->freeslots(temp, 1);

	if (is_temp) {
		gen.funcptr->freeslots(target, size);
	}
}


GENERATEFUNC(dotexpression) {
	INIT(dotexpression);

	bool is_temp = size == 0;

	if (is_temp) {
		size = 1;
		target = gen.funcptr->getslots(size);
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.funcptr->getslots(1);
	expressions::stringexpression index(expr.index);
	gen.runexpressionhandler(&index, temp, 1);
	gen.emit(prototype::OP_INDEX, target, target, temp);
	gen.funcptr->freeslots(temp, 1);

	if (is_temp) {
		gen.funcptr->freeslots(target, size);
	}
}

GENERATEFUNC(nameexpression) {
	if (size == 0) {
		return;
	}

	INIT(nameexpression);

	if (gen.funcptr->hasvariable(expr.name)) {
		gen.mov(target, gen.funcptr->varlookup[expr.name], 1);
	}
	else if (auto constant = gen.findconstant(_expr, expr.name)) {
		gen.runexpressionhandler(constant, target, 1);
	}
	/*
	else if(gen.funcptr->hasupvalue(expr.name)) {
		// TODO
	} */
	else if (expr.name == "_ENV") {
		gen.emit(prototype::OP_ENVIRONMENT, target);
	}
	else {
		gen.emit(prototype::OP_ENVIRONMENTGET, target, gen.add(expr.name));
	}
}

GENERATEFUNC(functioncallexpression) {
	INIT(functioncallexpression);
	auto &arglist = *dynamic_cast<args *>(expr.arglist);

	std::uint32_t stacksize = std::max(size, static_cast<std::uint32_t>(arglist.arglist.size() + 1 + (expr.methodname ? 1 : 0)));

	bool using_temp = stacksize > size || target == -1;
	auto functionindex = target;
	if (using_temp) {
		functionindex = gen.funcptr->getslots(stacksize);
	}
	auto argsindex = functionindex + 1;

	if (expr.methodname) {
		gen.runexpressionhandler(expr.funcexpr, argsindex, 1);
		expressions::stringexpression method(*expr.methodname);
		gen.runexpressionhandler(&method, functionindex, 1);
		gen.emit(prototype::OP_INDEX, functionindex, argsindex, functionindex);

		argsindex++;
	}
	else {
		gen.runexpressionhandler(expr.funcexpr, functionindex, 1);
	}

	auto opcode = prototype::OP_CALL;

	auto argsize = arglist.arglist.size();

	for (int i = 0; i < arglist.arglist.size(); i++) {
		auto &arg = arglist.arglist[i];	
		if (dynamic_cast<expressions::varargexpression *>(arg) && i == arglist.arglist.size() - 1) {
			opcode = prototype::OP_CALLV;
			argsize--;
		}
		else if (auto call = dynamic_cast<expressions::functioncallexpression *>(arg) && i == arglist.arglist.size() - 1) {
			opcode = prototype::OP_CALLM;
			gen.runexpressionhandler(arg, -1, 0);
			argsize--;
		}
		else {
			gen.runexpressionhandler(arg, argsindex + i, 1);
		}
	}

	gen.emit(opcode, functionindex, argsize, target == -1 ? 0 : size);

	if (using_temp) {
		if (target != -1 && size != 0) {
			gen.mov(target, functionindex, size);
		}
		gen.funcptr->freeslots(functionindex, stacksize);
	}
}

static void populate_tablevalue(prototype::_tablevalue *val, bytecodegenerator &gen, node *_expr) {
	if (auto numvalue = dynamic_cast<expressions::numberexpression *>(_expr)) {
		val->set_type(prototype::_tablevalue::NUMBER);
		val->set_index(gen.add(numvalue->data));
	}
	else if (auto strvalue = dynamic_cast<expressions::stringexpression *>(_expr)) {
		val->set_type(prototype::_tablevalue::STRING);
		val->set_index(gen.add(strvalue->data));
	}
	else if (auto strvalue = dynamic_cast<expressions::trueexpression *>(_expr)) {
		val->set_type(prototype::_tablevalue::CONSTANT);
		val->set_index(0);
	}
	else if (auto strvalue = dynamic_cast<expressions::falseexpression *>(_expr)) {
		val->set_type(prototype::_tablevalue::CONSTANT);
		val->set_index(1);
	}
	else if (auto strvalue = dynamic_cast<expressions::nilexpression *>(_expr)) {
		val->set_type(prototype::_tablevalue::CONSTANT);
		val->set_index(2);
	}
	else {
		throw exception(string("cannot create tablevalue for ") + _expr->tostring());
	}
}

GENERATEFUNC(tableexpression) {
	INIT(tableexpression);
	auto bcid = gen.funcptr->proto->tables_size();
	gen.emit(prototype::OP_TABLE, target, bcid);
	auto bcobj = gen.funcptr->proto->add_tables();
	for (auto &hashpart : expr.hashpart) {
		auto bckv = bcobj->add_hashpart();
		populate_tablevalue(&bckv->key, gen, hashpart.first);
		populate_tablevalue(&bckv->value, gen, hashpart.second);
	}

	for (auto &arraypart : expr.arraypart) {
		populate_tablevalue(bcobj->add_arraypart(), gen, arraypart);
	}
}

GENERATEFUNC(functionexpression) {
	if (size == 0) {
		return;
	}
	INIT(functionexpression);
	gen.emit(prototype::OP_FNEW, target, gen.protomap[_expr]);
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
#include "expressions.hpp"
#include "generator.hpp"
#include "parser/expressions.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser;
using namespace lorelai::bytecode;

static void generate_numberexpression(bytecodegenerator &gen, node &expr, std::uint32_t target, std::uint32_t size) {
	if (size == 0) {
		return;
	}
	gen.emit(bytecode::instruction_opcode_NUMBER, target, gen.add(dynamic_cast<expressions::numberexpression *>(&expr)->data));
}

static void generate_stringexpression(bytecodegenerator &gen, node &expr, std::uint32_t target, std::uint32_t size) {
	if (size == 0) {
		return;
	}
	gen.emit(bytecode::instruction_opcode_STRING, target, gen.add(dynamic_cast<expressions::stringexpression *>(&expr)->data));
}

static void generate_enclosedexpression(bytecodegenerator &gen, node &expr, std::uint32_t target, std::uint32_t size) {
	auto &child = *dynamic_cast<expressions::enclosedexpression *>(&expr)->children[0].get();
	gen.runexpressionhandler(child, target, size > 0 ? 1 : 0);
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
				auto temp = gen.curfunc.gettemp();
				gen.runexpressionhandler(expr.lhs, temp, 1);
				gen.runexpressionhandler(expr.rhs, temp, 1);
				gen.curfunc.freetemp(temp);
			}
			else {
				auto temp = gen.curfunc.gettemp(2);
				gen.runexpressionhandler(expr.lhs, temp, 1);
				gen.runexpressionhandler(expr.rhs, temp + 1, 1);
				gen.emit(opcode, temp, temp, temp + 1);
				gen.curfunc.freetemp(temp, 2);
			}
		}
		else {
			std::uint32_t rhs_stack;
			bool free_right = false;
			gen.runexpressionhandler(expr.lhs, target, 1);

			if (free_right = !get(expr.rhs, &rhs_stack, false)) {
				rhs_stack = gen.curfunc.gettemp();
				gen.runexpressionhandler(expr.rhs, rhs_stack, 1);
			}

			gen.emit(binoplookup[expr.op], target, target, rhs_stack);

			if (free_right) {
				gen.curfunc.freetemp(rhs_stack);
			}
		}
	}

	bool get(std::shared_ptr<node> &expr, std::uint32_t *stackposout, bool leftside) {
		if (auto name = dynamic_cast<expressions::nameexpression *>(expr.get())) {
			if (auto scope = gen.curfunc.curscope->findvariablescope(name->name)) {
				*stackposout = scope->getvariableindex(name->name);
				return true;
			}
		}

		return false;
	}

public:
	bytecodegenerator &gen;
};

static void generate_binopexpression(bytecodegenerator &gen, node &_expr, std::uint32_t target, std::uint32_t size) {
	auto &expr = *dynamic_cast<expressions::binopexpression *>(&_expr);
	binopsimplifier simplify(gen, expr, target, size);
}

std::unordered_map<string, bytecode::instruction_opcode> unoplookup = {
	{ "not", bytecode::instruction_opcode_NOT },
	{ "!", bytecode::instruction_opcode_NOT },
	{ "#", bytecode::instruction_opcode_LENGTH },
	{ "-", bytecode::instruction_opcode_MINUS },
};

static void generate_unopexpression(bytecodegenerator &gen, node &_expr, std::uint32_t target, std::uint32_t size) {
	auto &expr = *dynamic_cast<expressions::unopexpression *>(&_expr);

	if (size > 0) {
		gen.runexpressionhandler(expr.expr, target, 1);
		gen.emit(unoplookup[expr.op], target, target);
	}
	else if (unoplookup[expr.op] != bytecode::instruction_opcode_NOT) {
		auto temp = gen.curfunc.gettemp();

		gen.runexpressionhandler(expr.expr, temp, 1);
		gen.emit(unoplookup[expr.op], temp, temp);

		gen.curfunc.freetemp(temp);
	}
	else {
		gen.runexpressionhandler(expr.expr, target, 0);
	}
}

static void generate_nilexpression(bytecodegenerator &gen, node &expr, std::uint32_t target, std::uint32_t size) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 2);
}

static void generate_falseexpression(bytecodegenerator &gen, node &expr, std::uint32_t target, std::uint32_t size) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 1);
}

static void generate_trueexpression(bytecodegenerator &gen, node &expr, std::uint32_t target, std::uint32_t size) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 0);
}

static void generate_indexexpression(bytecodegenerator &gen, node &_expr, std::uint32_t target, std::uint32_t size) {
	auto &expr = *dynamic_cast<expressions::indexexpression *>(&_expr);

	bool is_temp = size == 0;

	if (is_temp) {
		size = 1;
		target = gen.curfunc.gettemp(size);
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.curfunc.gettemp();
	gen.runexpressionhandler(expr.index, temp, 1);
	gen.emit(bytecode::instruction_opcode_INDEX, target, target, temp);
	gen.curfunc.freetemp(temp);

	if (is_temp) {
		gen.curfunc.freetemp(target, size);
	}
}

static void generate_dotexpression(bytecodegenerator &gen, node &_expr, std::uint32_t target, std::uint32_t size) {
	auto &expr = *dynamic_cast<expressions::dotexpression *>(&_expr);
	bool is_temp = size == 0;

	if (is_temp) {
		size = 1;
		target = gen.curfunc.gettemp(size);
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.curfunc.gettemp();
	expressions::stringexpression name(expr.index->tostring());
	gen.runexpressionhandler(name, temp, 1);
	gen.emit(bytecode::instruction_opcode_INDEX, target, target, temp);
	gen.curfunc.freetemp(temp);

	if (is_temp) {
		gen.curfunc.freetemp(target, size);
	}
}

static void generate_nameexpression(bytecodegenerator &gen, node &_expr, std::uint32_t target, std::uint32_t size) {
	if (size == 0) {
		return;
	}

	auto &expr = *dynamic_cast<expressions::nameexpression *>(&_expr);
	if (auto scope = gen.curfunc.curscope->findvariablescope(expr.name)) {
		gen.emit(bytecode::instruction_opcode_MOV, target, scope->getvariableindex(expr.name), 1);
	}
	else if(gen.curfunc.hasupvalue(expr.name)) {
		// TODO
	}
	else if (expr.name == "_ENV" || expr.name == "_G") {
		gen.emit(bytecode::instruction_opcode_ENVIRONMENT, target);
	}
	else {
		gen.emit(bytecode::instruction_opcode_ENVIRONMENTGET, target, gen.add(expr.name));
	}
}

static void generate_functioncallexpression(bytecodegenerator &gen, node &_expr, std::uint32_t target, std::uint32_t size) {
	auto &expr = *dynamic_cast<expressions::functioncallexpression *>(&_expr);
	auto &arglist = *dynamic_cast<args *>(expr.arglist.get());

	size_t stacksize = std::max(size, static_cast<std::uint32_t>(arglist.children.size() + 1 + (expr.methodname ? 1 : 0)));

	bool using_temp = stacksize > size || target == -1;
	auto functionindex = target;
	if (using_temp) {
		functionindex = gen.curfunc.gettemp(stacksize);
	}
	auto argsindex = functionindex + 1;

	if (expr.methodname) {
		gen.runexpressionhandler(expr.funcexpr, argsindex, 1);
		expressions::stringexpression name(expr.methodname->tostring());
		gen.runexpressionhandler(name, functionindex, 1);
		gen.emit(bytecode::instruction_opcode_INDEX, functionindex, argsindex, functionindex);

		argsindex++;
	}
	else {
		gen.runexpressionhandler(expr.funcexpr, functionindex, 1);
	}

	auto opcode = bytecode::instruction_opcode_CALL;

	auto argsize = arglist.children.size();

	for (int i = 0; i < arglist.children.size(); i++) {
		auto &arg = arglist.children[i];
		if (dynamic_cast<expressions::varargexpression *>(arg.get()) && i == arglist.children.size() - 1) {
			opcode = bytecode::instruction_opcode_CALLV;
			argsize--;
		}
		else if (auto call = dynamic_cast<expressions::functioncallexpression *>(arg.get()) && i == arglist.children.size() - 1) {
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
			gen.emit(bytecode::instruction_opcode_MOV, target, functionindex, size);
		}
		gen.curfunc.freetemp(functionindex, stacksize);
	}
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
	// ADD(functioncallexpression),
	ADD(indexexpression),
	ADD(dotexpression),
	ADD(nameexpression),
	ADD(functioncallexpression),
};
#undef ADD
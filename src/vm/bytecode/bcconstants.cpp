#include <typeinfo>
#include <cmath>
#include "bcconstants.hpp"
#include "generator.hpp"

using namespace lorelai;
using namespace lorelai::bytecode;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

using generator = node *(*)(bytecodegenerator &gen, node *_expr);

#define GENERATEFUNC(t) static node *generate_##t(bytecodegenerator &gen, node *_expr)
#define INIT(t) expressions::t &expr = dynamic_cast<expressions::t &>(*_expr)
#define SIMPLEGENERATE(t) GENERATEFUNC(t) { INIT(t); return new t(expr); }
extern std::unordered_map<std::type_index, generator> generators;

static node *rungenerator(bytecodegenerator &gen, node *expr) {
	auto found = generators.find(typeid(*expr));

	if (found == generators.end()) {
		return nullptr;
	}

	return found->second(gen, expr);
}

SIMPLEGENERATE(numberexpression);
SIMPLEGENERATE(nilexpression);
SIMPLEGENERATE(falseexpression);
SIMPLEGENERATE(trueexpression);
SIMPLEGENERATE(stringexpression);

GENERATEFUNC(enclosedexpression) {
	INIT(enclosedexpression);

	if (auto enclosed = rungenerator(gen, expr.enclosed)) {
		return new enclosedexpression(enclosed);
	}

	return nullptr;
}

GENERATEFUNC(unopexpression) {
	INIT(unopexpression);

	if (auto newexpr = rungenerator(gen, expr.expr)) {
		if (auto num = dynamic_cast<numberexpression *>(newexpr)) {
			if (expr.op == "-") {
				num->data = -num->data;
				return num;
			}
		}
		return new unopexpression(expr.op, newexpr);
	}

	return nullptr;
}

GENERATEFUNC(binopexpression) {
	INIT(binopexpression);

	if (auto lhs = rungenerator(gen, expr.lhs)) {
		if (auto rhs = rungenerator(gen, expr.rhs)) {
			if (auto nlhs = dynamic_cast<numberexpression *>(lhs)) {
				if (auto nrhs = dynamic_cast<numberexpression *>(rhs)) {
					if (expr.op == "+") {
						nlhs->data += nrhs->data;
						nrhs = nullptr;
					}
					else if (expr.op == "-") {
						nlhs->data -= nrhs->data;
						nrhs = nullptr;
					}
					else if (expr.op == "/") {
						nlhs->data /= nrhs->data;
						nrhs = nullptr;
					}
					else if (expr.op == "*") {
						nlhs->data *= nrhs->data;
						nrhs = nullptr;
					}
					else if (expr.op == "^") {
						nlhs->data = std::pow(nlhs->data, nrhs->data);
						nrhs = nullptr;
					}

					if (!nrhs) {
						delete rhs;
						return lhs;
					}
				}
			}

			return new binopexpression(lhs, expr.op, rhs);
		}
		else {
			delete lhs;
		}
	}

	return nullptr;
}

GENERATEFUNC(nameexpression) {
	INIT(nameexpression);
	auto var = gen.curscope->find(expr.name);

	auto constant = gen.constantmap[*var];
	if (!constant) {
		return nullptr; // should never happen?
	}

	return rungenerator(gen, constant);
}

#define ADD(x) { typeid(expressions::x), generate_##x }
std::unordered_map<std::type_index, generator> generators = {
	ADD(numberexpression),
	ADD(nilexpression),
	ADD(falseexpression),
	ADD(trueexpression),
	ADD(stringexpression),
	ADD(enclosedexpression),
	ADD(binopexpression),
	ADD(unopexpression),
	ADD(nameexpression),
};
#undef ADD

node *bytecode::collapseconstant(bytecodegenerator &gen, node &expr) {
	return rungenerator(gen, &expr);
}
#include "expressions.hpp"
#include "lexer.hpp"
#include "visitor.hpp"
#include <sstream>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::expressions;

LORELAI_ACCEPT_BRANCH(unopexpression)

string unopexpression::tostring() {
	std::stringstream stream;
	stream << op << expr->tostring();
	return stream.str();
}
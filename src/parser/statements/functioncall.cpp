#include "statements.hpp"
#include "visitor.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::parser::statements;
using namespace lorelai::parser::expressions;

LORELAI_VISIT_BRANCH_DEFINE(functioncallstatement)
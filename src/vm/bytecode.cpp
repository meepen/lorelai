#include "bytecode.hpp"
#include "bytecode/generator.hpp"
#include "funcbody.hpp"
#include "bytecode/function.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::bytecode;


/*
	NOT DONE:
	fn(lorelai::parser::statements::localfunctionstatement)
	fn(lorelai::parser::statements::functionstatement)
*/

prototype lorelai::bytecode::create(chunk &data) {
	node *n = &data;

	bytecodegenerator generator;
	data.accept(generator, n);

	return generator.funcptr->finalize();
}

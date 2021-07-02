#include "bytecode.hpp"
#include "bytecode/generator.hpp"

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::bytecode;


/*
	NOT DONE:
	fn(lorelai::parser::statements::localfunctionstatement)
	fn(lorelai::parser::statements::functionstatement)
*/


std::shared_ptr<prototype> lorelai::bytecode::create(chunk &data) {
	bytecodegenerator generator;
	auto r = generator.curfunc.proto;
	std::shared_ptr<node> container = std::make_shared<chunk>(data);

	data.accept(generator, container);

	r->set_stacksize(generator.curfunc.funcstack.maxsize);

	return r;
}

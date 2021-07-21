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


class datagen : public variablevisitor {
public:
	using visitor::visit;
	using visitor::postvisit;

	LORELAI_VISIT_FUNCTION(funcbody) {
		variablevisitor::visit(obj, container);
		protomap[container] = idlist.back()++;

		idlist.push_back(0);

		return false;
	}
	LORELAI_POSTVISIT_FUNCTION(funcbody) {
		variablevisitor::postvisit(obj, container);
		idlist.pop_back();
	}

public:
	std::vector<std::uint32_t> idlist { 0 };
	std::unordered_map<node *, std::uint32_t> protomap;
};

prototype *lorelai::bytecode::create(chunk &data) {
	node *n = &data;
	datagen gen;
	data.accept(gen, n);

	bytecodegenerator generator(gen.protomap, gen);
	data.accept(generator, n);

	return generator.funcptr->release();
}

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


class protogen : public visitor {
public:
	using visitor::visit;
	using visitor::postvisit;

	LORELAI_VISIT_FUNCTION(funcbody) {
		protomap[container] = idlist.back()++;

		idlist.push_back(0);

		return false;
	}
	LORELAI_POSTVISIT_FUNCTION(funcbody) {
		idlist.pop_back();
	}

public:
	std::vector<std::uint32_t> idlist { 0 };
	std::unordered_map<node *, std::uint32_t> protomap;
};

std::shared_ptr<prototype> lorelai::bytecode::create(chunk &data) {
	node *n = &data;
	protogen protomap;
	data.accept(protomap, n);

	bytecodegenerator generator(protomap.protomap);
	data.accept(generator, n);

	auto r = generator.curfunc.proto;
	r->set_stacksize(generator.curfunc.maxsize);

	return generator.curfunc.proto;
}

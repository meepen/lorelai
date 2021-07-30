#include "bytecode.hpp"
#include "software.hpp"
#include "object.hpp"
#include "libraries.hpp"

using namespace lorelai;
using namespace lorelai::vm;

std::shared_ptr<state> softwarestate::create() {
	return std::make_shared<softwarestate>();
}

std::shared_ptr<state> state::create() {
	return softwarestate::create();
}


#ifdef LORELAI_SOFTWARE_FASTEST

std::shared_ptr<state> state::createfastest() {
	return softwarestate::create();
}

#endif // LORELAI_X86_FASTEST

void softwarestate::initlibs() {
	for (size_t i = 0; i < sizeof(libraries) / sizeof(*libraries); i++) {
		const auto &namedlib = libraries[i];

		auto libt = namedlib.name ? tableobject::create(*this) : registry;
		if (namedlib.name) {
			registry.rawset(*this, stringobject::create(*this, namedlib.name), libt);
		}

		auto lib = namedlib.lib;
		while (lib->name) {
			libt.rawset(*this, stringobject::create(*this, lib->name), cfunctionobject::create(*this, lib->func));
			lib++;
		}
	}
}

int softwarestate::call(int nargs, int nrets) {
	stack.stacktop -= nargs + 1;
	auto oldtop = stack.pushstack(nargs + 1);
	auto ret = stack[0].call(*this, nargs);
	auto rets = stack.popstack(oldtop, ret);
	if (nrets == -1) {
		nrets = rets;
	}

	for (int i = rets; i < nrets; i++) {
		stack.stacktop[i].unset();
	}

	stack.stacktop += nrets;

	return rets;
}

softwarestate::softwarestate() : stack(this) {
	registry = tableobject::create(*this);
	initlibs();
}

void softwarestate::loadfunction(const bytecode::prototype &code) {
	(stack.stacktop++)->set(luafunctionobject::create(*this, code));
} 
void softwarestate::loadnumber(number num) {
	(stack.stacktop++)->set(num);
}
void softwarestate::loadstring(string str) {
	(stack.stacktop++)->set(stringobject::create(*this, str));
}

softwarestate::~softwarestate() {
	for (auto item : memory) {
		switch (item->type) {
		case STRING:
			item->get<stringobject>()->~stringobject();
			break;
		case TABLE:
			item->get<tableobject>()->~tableobject();
			break;
		case LUAFUNCTION:
			item->get<luafunctionobject>()->~luafunctionobject();
			break;
		case CFUNCTION:
			item->get<cfunctionobject>()->~cfunctionobject();
			break;
		default:
			throw;
		}
	}
}
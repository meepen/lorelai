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

state::_retdata softwarestate::call(int nargs, int nrets) {
	auto old = stack.pushpointer(stack.base + stack.top - nargs - 1);
	auto tocall = stack[0];
	auto rets = stack.poppointer(old, tocall.call(*this, nargs, nrets), stack.base + stack.top - nargs - 1, nrets);

	return {
		stack.base + stack.top - nargs - 1 + nrets,
		rets
	};
}

softwarestate::softwarestate() : stack(this) {
	registry = tableobject::create(*this);
	initlibs();
}

void softwarestate::loadfunction(std::shared_ptr<bytecode::prototype> code) {
	stack[stack.top++].set(luafunctionobject::create(*this, code));
} 
void softwarestate::loadnumber(number num) {
	stack[stack.top++].set(num);
}
void softwarestate::loadstring(string str) {
	stack[stack.top++].set(stringobject::create(*this, str));
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
			std::cerr << "unknown gc object type " << std::to_string(item->type) << " ptr: " << item << std::endl;
		}
	}
}
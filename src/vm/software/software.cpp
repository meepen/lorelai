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
		auto &namedlib = libraries[i];

		auto libt = namedlib.name ? tableobject::create(*this) : registry;
		if (namedlib.name) {
			registry->rawset(*this, stringobject::create(*this, namedlib.name), libt);
		}

		auto lib = namedlib.lib;
		while (lib->name) {
			libt->rawset(*this, stringobject::create(*this, lib->name), cfunctionobject::create(*this, lib->func));
			lib++;
		}
	}
}

state::_retdata softwarestate::call(int nargs, int nrets) {
	auto old = stack.pushpointer(stack.base + stack.top - nargs - 1);
	auto tocall = stack[0];
	auto rets = stack.poppointer(old, tocall->call(*this, nrets, nargs), stack.base + stack.top - nargs - 1, nrets);

	return {
		stack.base + stack.top - nargs - 1 + nrets,
		rets
	};
}

int softwarestate::_stack::poppointer(const stackpos old, const state::_retdata retdata, int to, int amount) {
	if (amount == -1) {
		amount = retdata.retsize;
	}

	for (int i = 1; i <= std::min(amount, retdata.retsize); i++) {
		data[to + i - 1] = data[retdata.retbase + i - 1];
	}

	for (int i = std::min(amount, retdata.retsize) + 1; i <= amount; i++) {
		data[to + i - 1] = nilobject::create(*st);
	}

	top = old.top;
	base = old.base;

	return amount;
}

softwarestate::softwarestate() {
	initallocators();
	registry = tableobject::create(*this);
	initlibs();
}

void softwarestate::initallocators() {
	trueobj = boolallocator.take(true);
	falseobj = boolallocator.take(false);

	nil = nilallocator.take();
}

void softwarestate::loadfunction(std::shared_ptr<bytecode::prototype> code) {
	stack[stack.top++] = luafunctionobject::create(*this, code);
} 
void softwarestate::loadnumber(number num) {
	stack[stack.top++] = numberobject::create(*this, num);
}

void softwarestate::_stack::initstack() {
	for (auto &stackpos : data) {
		stackpos = nilobject::create(*st);
	}
}
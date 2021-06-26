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

		auto target = registry;
		if (namedlib.name) {
			auto newtarget = std::make_shared<tableobject>();
			target->rawset(*this, std::make_shared<stringobject>(namedlib.name), newtarget);
			target = newtarget;
		}

		auto lib = namedlib.lib;
		while (lib->name) {
			target->rawset(*this, std::make_shared<stringobject>(lib->name), std::make_shared<cfunctionobject>(lib->func));
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

template <int size>
int softwarestate::_stack<size>::poppointer(const stackpos old, const state::_retdata retdata, int to, int amount) {
	if (amount == -1) {
		amount = retdata.retsize;
	}

	for (int i = 1; i <= std::min(amount, retdata.retsize); i++) {
		data[to + i - 1] = data[retdata.retbase + i - 1];
	}

	for (int i = std::min(amount, retdata.retsize) + 1; i <= amount; i++) {
		data[to + i - 1] = std::make_shared<nilobject>();
	}

	top = old.top;
	base = old.base;

	return amount;
}
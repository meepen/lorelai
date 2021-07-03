#include "../libraries.hpp"
#include "../software.hpp"
#include "../object.hpp"

#include <iostream>

using namespace lorelai;
using namespace lorelai::vm;

static int print(softwarestate &state, int nargs, int nrets) {
	for (int i = 1; i <= nargs; i++) {
		if (i > 1) {
			std::cout << "    ";
		}
		std::cout << state[i].tostring(state);
	}
	std::cout << std::endl;

	return 0;
}

static int rawset(softwarestate &state, int nargs, int nrets) {
	auto stackptr = &state[0];
	stackptr[1].rawset(state, stackptr[2], stackptr[3]);

	return 0;
}

static int Lselect(softwarestate &state, int nargs, int nrets) {
	auto amount = state[1].tonumber(state);

	return std::max(nargs - 1, (int)amount);
}

static int tostring(softwarestate &state, int nargs, int nrets) {
	state.loadstring(state[1].tostring(state));

	return 1;
}

static int type(softwarestate &state, int nargs, int nrets) {
	state.loadstring(state[1].gettypename());

	return 1;
}

library vm::global[] = {
	{ "print", print },
	{ "rawset", rawset },
	{ "select", Lselect },
	{ "tostring", tostring },
	{ "type", type },
	{ nullptr, nullptr }
};
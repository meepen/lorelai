#include "../libraries.hpp"
#include "../software.hpp"
#include "../object.hpp"

#include <iostream>

using namespace lorelai;
using namespace lorelai::vm;

// NYI:
// unpack
// _VERSION
// _G
// rawequal
// loadstring
// loadfile
// load
// ipairs
// pairs
// next
// getmetatable
// setmetatable
// getfenv
// dofile
// collectgarbage

static int Lassert(softwarestate &state, int nargs, int nrets) {
	if (not state[1].tobool(state)) {
		throw exception(nargs >= 2 ? state[2].tostring(state) : "assertion failed!");
	}

	return 0;
}

static int Lerror(softwarestate &state, int nargs, int nrets) {
	throw exception(nargs >= 1 ? state[1].tostring(state) : "expected 'string' for argument #1 to 'error'");
	return 0;
}

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
	{ "assert", Lassert },
	{ "error", Lerror },
	{ "print", print },
	{ "rawset", rawset },
	{ "select", Lselect },
	{ "tostring", tostring },
	{ "type", type },
	{ nullptr, nullptr }
};
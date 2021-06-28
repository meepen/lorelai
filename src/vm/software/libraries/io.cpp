#include "../libraries.hpp"
#include "../software.hpp"
#include "../object.hpp"

#include <iostream>

using namespace lorelai;
using namespace lorelai::vm;

static int io_write(softwarestate &state, int nargs, int nrets) {
	for (size_t i = 1; i <= nargs; i++) {
		std::cout << state[i].tostring(state);
	}

	return 0;
}

library vm::io[] = {
	{ "write", io_write },
	{ nullptr, nullptr }
};
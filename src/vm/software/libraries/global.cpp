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

library vm::global[] = {
	{ "print", print },
	{ nullptr, nullptr }
};
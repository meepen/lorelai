#include "../libraries.hpp"

#include <iostream>

using namespace lorelai;
using namespace lorelai::vm;

size_t print(softwarestate &state, objectcontainer *out, size_t nrets, size_t nargs) {
	for (size_t i = 1; i <= nargs; i++) {
		std::cout << out[i]->tostring(state, out[nargs]);
	}
	std::cout << std::endl;

	return 0;
}

library vm::global[] = {
	{ "print", print },
	{ nullptr, nullptr }
};
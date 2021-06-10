#include "jit.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	lorelai::jit::state lua;
	auto func = lua.compile();

	std::cout << func(1, nullptr) << std::endl;
	return 0;
}
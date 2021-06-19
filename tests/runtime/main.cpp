#include "state.hpp"
#include "object.hpp"
#include "bytecode.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	auto bytecode = lorelai::vm::parse("local a = 3.14");

	return 0;
}

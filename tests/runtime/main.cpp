#include "state.hpp"
#include "object.hpp"
#include "bytecode.hpp"
#include <iostream>

#ifdef __linux__
#include <cxxabi.h>
template <typename T>
std::string gettypename(T &data) {
    auto ptr = std::unique_ptr<char, decltype(& std::free)>{
        abi::__cxa_demangle(typeid(data).name(), nullptr, nullptr, nullptr),
        std::free
    };
    return {ptr.get()};
}
#else
template <typename T>
std::string gettypename(T &data) {
	return typeid(data).name();
}
#endif

static void print_branch(size_t idx, lorelai::parser::node &node) {
	for (size_t i = 0; i < idx; i++) {
		std::cout << "  ";
	}

	std::cout << gettypename(node) << std::endl;

	try {
		lorelai::parser::branch &branch = dynamic_cast<lorelai::parser::branch &>(node);
		for (auto &child : branch.children) {
			print_branch(idx + 1, *child);
		}
	}
	catch (std::exception &e) {
		
	}
}

int main(int argc, char *argv[]) {
	lorelai::parser::chunk mainchunk("local a = 3.14");
	print_branch(0, mainchunk);
	auto bytecode = lorelai::vm::parse(mainchunk);

	std::cout << "Bytecode evaluated: " << std::endl;
	std::cout << "    Instructions: " << bytecode.instructions_size() << std::endl;
	std::cout << "    Prototypes:   " << bytecode.protos_size() << std::endl;
	std::cout << "    Numbers:      " << bytecode.numbers_size() << std::endl;
	std::cout << "    Strings:      " << bytecode.strings_size() << std::endl;

	return 0;
}

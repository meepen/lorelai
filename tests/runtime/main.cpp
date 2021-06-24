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
	std::string inputcode = R"(
		local a = -3.14, (false ~= not true), true, nil, 333, 'bbbd', ('b').c, d['e'].f(), g:h(i)
		a, b, c().d = 4, "B"
		e(1)
)";

	lorelai::parser::chunk mainchunk(inputcode);
	print_branch(0, mainchunk);
	auto bytecode = lorelai::vm::parse(mainchunk);

	std::cout << "Code evaluated: " << inputcode << std::endl;

	std::cout << "Bytecode evaluated: " << std::endl;
	std::cout << "    Instructions: " << bytecode.instructions_size() << std::endl;
	std::cout << "    Prototypes:   " << bytecode.protos_size() << std::endl;
	std::cout << "    Numbers:      " << bytecode.numbers_size() << std::endl;
	for (int i = 0; i < bytecode.numbers_size(); i++) {
		std::cout << "        #" << i << ": " << std::to_string(bytecode.numbers(i)) << std::endl;
	}
	std::cout << "    Strings:      " << bytecode.strings_size() << std::endl;
	for (int i = 0; i < bytecode.strings_size(); i++) {
		std::cout << "        #" << i << ": " << bytecode.strings(i) << std::endl;
	}

	size_t longest = 0;
	auto descriptor = lorelai::vm::bytecode::instruction_opcode_descriptor();
	for (int i = 0; i < descriptor->value_count(); i++) {
		longest = std::max(descriptor->value(i)->name().length() + 1, longest);
	}

	std::string max_index = std::to_string(bytecode.instructions_size() - 1);

	for (int i = 0; i < bytecode.instructions_size(); i++) {	
		auto &instruct = bytecode.instructions(i);
		auto instrname = lorelai::vm::bytecode::instruction_opcode_Name(instruct.op());

		std::string index = std::to_string(i);

		std::cout << "#" << index << std::string(max_index.size() - index.size() + 1, ' ') << "| " <<  instrname << std::string(longest - instrname.size(), ' ') << "| ";

		if (instruct.has_a()) {
			std::cout << instruct.a();
			if (instruct.has_b()) {
				std::cout << ", " << instruct.b();
				if (instruct.has_c()) {
					std::cout << ", " << instruct.c();
				}
			}
		}

		std::cout << std::endl;
	}

	return 0;
}

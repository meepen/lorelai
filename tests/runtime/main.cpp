#include "state.hpp"
#include "bytecode.hpp"
#include <iostream>
#include <fstream>
#include "tclap/CmdLine.h"
#include "tclap/ValueArg.h"

using namespace TCLAP;

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

class exception : public std::exception {
public:
	exception(std::string what) : _what(what) { }

	const char *what() const noexcept override {
		return _what.c_str();
	}

	std::string _what;
};

static std::string getcode(ValueArg<std::string> &filename, ValueArg<std::string> &code) {
	if (code.isSet()) {
		return code.getValue();
	}
	else if (filename.isSet()) {
		std::ifstream luafile(filename.getValue());
		if (!luafile.good()) {
			throw exception("file does not exist");
		}
		return std::string(
			std::istreambuf_iterator<char>(luafile),
			std::istreambuf_iterator<char>()
		);
	}
	else {
		throw exception("no code provided");
	}
}

static void printproto(lorelai::bytecode::prototype &bytecode) {
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
	auto descriptor = lorelai::bytecode::instruction_opcode_descriptor();
	for (int i = 0; i < descriptor->value_count(); i++) {
		longest = std::max(descriptor->value(i)->name().length() + 1, longest);
	}

	std::string max_index = std::to_string(bytecode.instructions_size() - 1);

	for (int i = 0; i < bytecode.instructions_size(); i++) {	
		auto &instruct = bytecode.instructions(i);
		auto instrname = lorelai::bytecode::instruction_opcode_Name(instruct.op());

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
}

int main(int argc, char *argv[]) {
	
	try {
		// Define the command line object.
		CmdLine cmd("lorelai", ' ', "0.0");
		ValueArg<std::string> filename("f", "file", "File to load code from", false, "-", "string");
		cmd.add(filename);
		ValueArg<std::string> code("l", "lua", "Lua code to load", false, "", "string");
		cmd.add(code);

		SwitchArg bytecodeonly("b", "bytecode", "Print bytecode", false);
		cmd.add(bytecodeonly);
		SwitchArg raw("r", "raw", "Raw output", false);
		cmd.add(raw);
		SwitchArg astonly("a", "ast", "Print ast structure", false);
		cmd.add(astonly);

		cmd.parse(argc, argv);

		auto luacode = getcode(filename, code);
		lorelai::parser::chunk mainchunk(luacode);
		if (astonly.getValue()) {
			print_branch(0, mainchunk);
			return 0;
		}

		auto bytecode = lorelai::bytecode::create(mainchunk);

		if (bytecodeonly.getValue()) {
			if (raw.getValue()) {
				std::cout << bytecode->SerializeAsString();
				return 0;
			}

			printproto(*bytecode);
			return 0;
		}

		auto state = lorelai::vm::state::create();
		state->loadfunction(bytecode);
		state->call(0, 0);
	}
	catch (ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return 1;
	}
	catch (std::exception &e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

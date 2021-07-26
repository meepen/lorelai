#include "state.hpp"
#include "bytecode.hpp"
#include <iostream>
#include <fstream>
#include "tclap/CmdLine.h"
#include "tclap/ValueArg.h"
#include "bytecode/scope.hpp"

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
	std::cout << std::string(idx * 2, ' ');

	std::cout << gettypename(node) << std::endl;

	try {
		lorelai::parser::branch &branch = dynamic_cast<lorelai::parser::branch &>(node);
		for (auto &child : branch.getchildren()) {
			print_branch(idx + 1, **child);
		}
	}
	catch (std::exception &e) { }
}

static void print_scopes(std::vector<std::shared_ptr<lorelai::bytecode::scope>> &list, std::shared_ptr<lorelai::bytecode::scope> with_parent = nullptr, size_t idx = 0) {
	auto tab = std::string(idx * 4, ' ');
	std::vector<std::shared_ptr<lorelai::bytecode::scope>> found;

	for (auto &child : list) {
		if (child->parent == with_parent) {
			found.push_back(child);
		}
	}

	for (auto &child : found) {
		std::cout << tab << "scope #" << child->id << " (" << child->variables.size() << ")" << std::endl;

		for (auto &var : child->variables) {
			std::cout << tab << var.name << " accesses:" << var.accesses << " writes:" << var.writes << std::endl;
		}
		print_scopes(list, child, idx + 1);
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
		std::cout << "        #" << i << ": " << std::to_string(bytecode.number(i)) << std::endl;
	}
	std::cout << "    Strings:      " << bytecode.strings_size() << std::endl;
	for (int i = 0; i < bytecode.strings_size(); i++) {
		std::cout << "        #" << i << ": " << bytecode.string(i) << std::endl;
	}

	std::string max_index = std::to_string(bytecode.instructions_size() - 1);

	for (int i = 0; i < bytecode.instructions_size(); i++) {	
		auto &instruct = bytecode.instruction(i);
		auto instrname = lorelai::bytecode::opcode_names[instruct->opcode];

		std::string index = std::to_string(i);

		std::cout << "#" << index << std::string(max_index.size() - index.size() + 1, ' ') << "| " <<  instrname << " | ";

		std::cout << instruct->a << ", " << instruct->b << ", " << instruct->c << std::endl;
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

		SwitchArg bytecodeonly("b", "print-bytecode", "Print bytecode", false);
		cmd.add(bytecodeonly);
		SwitchArg raw("r", "raw", "Raw output", false);
		cmd.add(raw);
		SwitchArg astonly("a", "print-ast", "Print ast structure", false);
		cmd.add(astonly);

		SwitchArg scopesonly("s", "print-scopes", "Print scope structure for variables", false);
		cmd.add(scopesonly);

		cmd.parse(argc, argv);

		auto luacode = getcode(filename, code);
		lorelai::parser::chunk mainchunk(luacode);
		if (astonly.getValue()) {
			print_branch(0, mainchunk);
			return 0;
		}

		if (scopesonly.getValue()) {
			auto map = lorelai::bytecode::generatescopemap(&mainchunk);

			print_scopes(map.scopes);

			return 0;
		}

		std::unique_ptr<lorelai::bytecode::prototype> bytecode(lorelai::bytecode::create(mainchunk));

		if (bytecodeonly.getValue()) {
			if (raw.getValue()) {
				throw; // std::cout << bytecode->SerializeAsString();
				return 0;
			}

			printproto(*bytecode);
			return 0;
		}

		auto state = lorelai::vm::state::create();
		state->loadfunction(*bytecode.get());
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

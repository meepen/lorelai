#include <fstream>
#include <streambuf>
#include <string>
#include <exception>
#include <iostream>
#include <typeinfo>
#include "parser.hpp"
#include "parser/statements.hpp"
#include "parser/node.hpp"
#include "visitor.hpp"

/*
#include "json.hpp"
using json = nlohmann::json;
*/

using namespace lorelai;
using namespace lorelai::parser;

#define BASIC_TEST(classname) \
	LORELAI_VISIT_FUNCTION(classname) { \
		visited++; \
		std::cout << "visited: " << #classname << std::endl; \
		return false; \
	} \
	LORELAI_POSTVISIT_FUNCTION(classname) { \
		postvisited++; \
		std::cout << "post visit: " << #classname << std::endl; \
	}

class visitor_printer : public visitor {
	LORELAI_VISIT_NAME_MACRO(BASIC_TEST)

public:
	size_t visited = 0, postvisited = 0;
};

class printer_visitor : public visitor {
	LORELAI_VISIT_FUNCTION(expressions::stringexpression) {
		std::cout << "STRING: " << obj.data << std::endl;
		return false;
	}
	LORELAI_VISIT_FUNCTION(expressions::numberexpression) {
		std::cout << "NUMBER: " << obj.data << std::endl;
		return false;
	}
};

class number_to_false_visitor : public visitor {
	LORELAI_VISIT_FUNCTION(expressions::numberexpression) {
		delete container;
		container = new expressions::falseexpression();
		std::cout << "REPLACED!!" << std::endl;
		return false;
	}
};

class return_delete_visitor : public visitor {
	LORELAI_VISIT_FUNCTION(statements::returnstatement) {
		std::cout << "DELETED!" << std::endl;
		return true;
	}
};

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
		for (auto &child : branch.getchildren()) {
			print_branch(idx + 1, **child);
		}
	}
	catch (std::exception &e) {
		
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Error: no argument for file" << std::endl;
		return 1;
	} 

	std::ifstream luafile(argv[1]);
	auto luacode = std::string(
		std::istreambuf_iterator<char>(luafile),
		std::istreambuf_iterator<char>()
	);

	try {
		chunk main(luacode);
		node *container = &main;
		visitor_printer printer;
		main.accept(printer, container);
		if (printer.postvisited != printer.visited) {
			std::cerr << "postvisited (" << printer.postvisited << ") != visited (" << printer.visited << ")" << std::endl;
			return 1;
		}

		printer_visitor string_printer;
		main.accept(string_printer, container);
		print_branch(0, main);

		number_to_false_visitor replacer;
		main.accept(replacer, container);
		print_branch(0, main);

		return_delete_visitor deleter;
		main.accept(deleter, container);
		print_branch(0, main);

		std::cout << "success (visit)" << std::endl;
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

#include <fstream>
#include <streambuf>
#include <string>
#include <exception>
#include <iostream>
#include <typeinfo>
#include "astgen.hpp"
#include "astgen/statements.hpp"
#include "astgen/node.hpp"
#include "visitor.hpp"

/*
#include "json.hpp"
using json = nlohmann::json;
*/

using namespace lorelai;
using namespace lorelai::astgen;

#define BASIC_TEST(classname) \
	bool visit(classname &_node, std::shared_ptr<node> &container) override { \
		std::cout << "visited: " << #classname << std::endl; \
		return false; \
	}
class visitor_printer : public visitor {
	LORELAI_VISIT_NAME_MACRO(BASIC_TEST)
};

class printer_visitor : public visitor {
	bool visit(expressions::stringexpression &node, std::shared_ptr<lorelai::astgen::node> &container) override {
		std::cout << "STRING: " << node.data << std::endl;
		return false;
	}
	bool visit(expressions::numberexpression &node, std::shared_ptr<lorelai::astgen::node> &container) override {
		std::cout << "NUMBER: " << node.data << std::endl;
		return false;
	}
};

class number_to_false_visitor : public visitor {
	bool visit(expressions::numberexpression &node, std::shared_ptr<lorelai::astgen::node> &container) override {
		container = std::make_shared<expressions::falseexpression>();
		std::cout << "REPLACED!!" << std::endl;
		return false;
	}
};

class return_delete_visitor : public visitor {
	bool visit(statements::returnstatement &node, std::shared_ptr<lorelai::astgen::node> &container) override {
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

static void print_branch(size_t idx, lorelai::astgen::node &node) {
	for (size_t i = 0; i < idx; i++) {
		std::cout << "  ";
	}

	std::cout << gettypename(node) << std::endl;

	try {
		lorelai::astgen::branch &branch = dynamic_cast<lorelai::astgen::branch &>(node);
		for (auto &child : branch.children) {
			print_branch(idx + 1, *child);
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
		std::shared_ptr<node> main = std::make_shared<mainchunk>(luacode);
		visitor_printer printer;
		main->accept(printer, main);
		print_branch(0, *main);

		printer_visitor string_printer;
		main->accept(string_printer, main);
		print_branch(0, *main);

		number_to_false_visitor replacer;
		main->accept(replacer, main);
		print_branch(0, *main);

		return_delete_visitor deleter;
		main->accept(deleter, main);
		print_branch(0, *main);

		std::cout << "success (visit)" << std::endl;
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

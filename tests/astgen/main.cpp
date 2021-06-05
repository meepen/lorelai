#include <fstream>
#include <streambuf>
#include <string>
#include <exception>
#include <iostream>
#include <typeinfo>

#include "json.hpp"
#include "astgen.hpp"

using json = nlohmann::json;

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

	std::ifstream t(argv[1]);
	std::string results((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	try {
		auto data = json::parse(results);
		auto input_string = data["input"].get<std::string>();

		lorelai::astgen::mainchunk main(input_string);
		print_branch(0, main);

		std::cout << "success" << std::endl;
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
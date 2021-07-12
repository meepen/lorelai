#include <fstream>
#include <streambuf>
#include <string>
#include <exception>
#include <iostream>
#include <typeinfo>
#include <chrono>
#include "parser.hpp"


// uncomment later on when we add tests
/*
#include "json.hpp"
using json = nlohmann::json;
*/

#ifdef __linux__
#include <cxxabi.h>
template <typename T>
std::string gettypename(T &data) {
	auto name = typeid(data).name();
	auto demangled = abi::__cxa_demangle(name, nullptr, nullptr, nullptr);

	if (!demangled) {
		throw;
	}

    auto ptr = std::unique_ptr<char, decltype(& std::free)>{
        demangled,
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
		auto start = std::chrono::high_resolution_clock::now();
		lorelai::parser::chunk loadedast(luacode, true);
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		print_branch(0, loadedast);

		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		std::cout << "success in " << microseconds << " microseconds" << std::endl;
	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
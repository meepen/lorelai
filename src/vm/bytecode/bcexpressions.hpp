#ifndef BYTECODE_EXPRESSIONS_HPP_
#define BYTECODE_EXPRESSIONS_HPP_

#include <unordered_map>
#include <typeindex>
#include <cstdint>
#include "node.hpp"

namespace lorelai {
	namespace parser {
		class node;
	}

	namespace bytecode {
		class bytecodegenerator;
		using expressiongenerator = void (*)(bytecodegenerator &gen, const parser::node &expr, std::uint32_t stackindex, std::uint32_t size);

		extern std::unordered_map<std::type_index, expressiongenerator> expressionmap;
	}
}

#endif // BYTECODE_EXPRESSIONS_HPP
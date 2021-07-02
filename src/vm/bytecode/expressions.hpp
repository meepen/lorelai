#ifndef BYTECODE_EXPRESSIONS_HPP_
#define BYTECODE_EXPRESSIONS_HPP_

#include <unordered_map>
#include <typeindex>

namespace lorelai {
	namespace parser {
		class node;
	}

	namespace bytecode {
		class bytecodegenerator;
		using expressiongenerator = void (*)(bytecodegenerator &gen, parser::node &expr, size_t stackindex, size_t size);

		extern std::unordered_map<std::type_index, expressiongenerator> expressionmap;
	}
}

#endif // BYTECODE_EXPRESSIONS_HPP
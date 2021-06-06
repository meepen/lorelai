#ifndef ASTGEN_HPP_
#define ASTGEN_HPP_

#include <memory>
#include <vector>
#include <string>
#include <exception>

#include "astgen/expressions.hpp"
#include "astgen/errors.hpp"
#include "astgen/node.hpp"
#include "astgen/statements.hpp"
#include "lexer.hpp"

namespace lorelai {
	namespace astgen {
		class chunk : public branch {
		public:
			chunk(lexer &lex);

			bool accept(visitor &visit, std::shared_ptr<node> &container) override;
		};

		class mainchunk : public chunk {
		public:
			mainchunk(lexer data) : chunk(data) { }
		};
	}
}

#endif // ASTGEN_HPP_
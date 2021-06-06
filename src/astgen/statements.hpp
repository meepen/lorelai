#ifndef STATEMENTS_HPP_
#define STATEMENTS_HPP_

#define LORELAI_STATEMENT_CLASS_MACRO(fn) \
	fn(lorelai::astgen::statements::returnstatement) \
	fn(lorelai::astgen::statements::dostatement)

#include <vector>
#include <memory>
#include "lexer.hpp"
#include "types.hpp"
#include "node.hpp"
#include "expressions.hpp"

namespace lorelai {
	namespace astgen {
		class statement {
		public:
			static std::shared_ptr<node> read(lexer &lex);
		};

		namespace statements {
			class returnstatement : public branch, public statement {
			public:
				returnstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};
			
			class dostatement : public branch, public statement {
			public:
				dostatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};
		}
	}
}

#endif // STATEMENTS_HPP_
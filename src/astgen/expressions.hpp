#ifndef EXPRESSIONS_HPP_
#define EXPRESSIONS_HPP_

#define LORELAI_EXPRESSION_LITERAL_CLASS_MACRO(fn) \
	fn(lorelai::astgen::expressions::nilexpression) \
	fn(lorelai::astgen::expressions::falseexpression) \
	fn(lorelai::astgen::expressions::trueexpression) \
	fn(lorelai::astgen::expressions::numberexpression) \
	fn(lorelai::astgen::expressions::stringexpression) \
	fn(lorelai::astgen::expressions::varargexpression)

#include "types/types.hpp"
#include "node.hpp"
#include "lexer.hpp"

namespace lorelai {
	namespace astgen {
		class expression : public node {
		public:
			static std::shared_ptr<expression> read(lexer &lex);
		};
		namespace expressions {
			class nilexpression : public expression {
			public:
				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class falseexpression : public expression {
			public:
				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class trueexpression : public expression {
			public:
				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class numberexpression : public expression {
			private:
				numberexpression() { }
			public:
				numberexpression(number num) : data(num) { }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			public:
				number data;
			};

			class stringexpression : public expression {
			public:
				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			public:
				bool is_long_string = false;
				size_t long_string_length;
				string data;
			};

			class varargexpression : public expression {
			public:
				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			};
		}
	}
}

#endif // EXPRESSIONS_HPP_
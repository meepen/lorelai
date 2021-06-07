#ifndef EXPRESSIONS_HPP_
#define EXPRESSIONS_HPP_


#define LORELAI_EXPRESSION_NODES_CLASS_MACRO(fn) \
	fn(lorelai::astgen::expressions::nilexpression) \
	fn(lorelai::astgen::expressions::falseexpression) \
	fn(lorelai::astgen::expressions::trueexpression) \
	fn(lorelai::astgen::expressions::numberexpression) \
	fn(lorelai::astgen::expressions::stringexpression) \
	fn(lorelai::astgen::expressions::varargexpression) \
	fn(lorelai::astgen::expressions::nameexpression)

#define LORELAI_EXPRESSION_CLASS_MACRO(fn) \
	fn(lorelai::astgen::expressions::nilexpression) \
	fn(lorelai::astgen::expressions::falseexpression) \
	fn(lorelai::astgen::expressions::trueexpression) \
	fn(lorelai::astgen::expressions::numberexpression) \
	fn(lorelai::astgen::expressions::stringexpression) \
	fn(lorelai::astgen::expressions::varargexpression) \
	fn(lorelai::astgen::expressions::nameexpression) \
	fn(lorelai::astgen::expressions::tableexpression) \
	fn(lorelai::astgen::expressions::enclosedexpression)

#include "types.hpp"
#include "node.hpp"
#include <unordered_map>
#include <vector>

namespace lorelai {
	class lexer;
	namespace astgen {
		class expression {
		public:
			static std::shared_ptr<node> read(lexer &lex);
		};

		class prefixexpression : public expression {
		public:
			static std::shared_ptr<node> read(lexer &lex);
		};

		namespace expressions {
			class nilexpression : public node, public expression {
			public:
				nilexpression() { }
				nilexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class falseexpression : public node, public expression {
			public:
				falseexpression() { }
				falseexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class trueexpression : public node, public expression {
			public:
				trueexpression() { }
				trueexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class numberexpression : public node, public expression {
			public:
				numberexpression(lexer &lex);
				numberexpression(number num) : data(num) { }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				number data;
			};


			static const std::unordered_map<string::value_type, string> escape_chars = {
				{'v', "\v"},
				{'z', string(1, '\x00')},
				{'t', "\t"},
				{'a', "\a"},
				{'r', "\r"},
				{'n', "\n"},
				{'\\', "\\"},
				{'b', "\b"}
			};

			class stringexpression : public node, public expression {
			public:
				stringexpression(lexer &lex);

				stringexpression(string _data) : data(_data) { }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			public:
				using type = enum {
					SINGLE_QUOTE,
					DOUBLE_QUOTE,
					LITERAL,
					UNKNOWN
				};
				type string_type = UNKNOWN;
				size_t long_string_depth = 0;
				string data;
			};

			class varargexpression : public node, public expression {
			public:
				varargexpression() { }
				varargexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class nameexpression : public node, public expression {
			public:
				nameexpression(string data) : name(data) { }
				nameexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			public:
				string name;
			};

			class tableexpression : public branch, public expression {
			public:
				tableexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			public:
				std::vector<std::pair<std::shared_ptr<node>, std::shared_ptr<node>>> tabledata;
			};


			// prefix expressions
			class enclosedexpression : public branch, public prefixexpression {
			public:
				enclosedexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};
		}
	}
}

#endif // EXPRESSIONS_HPP_
#ifndef EXPRESSIONS_HPP_
#define EXPRESSIONS_HPP_


#define LORELAI_EXPRESSION_NODES_CLASS_MACRO(fn) \
	fn(lorelai::parser::expressions::nilexpression) \
	fn(lorelai::parser::expressions::falseexpression) \
	fn(lorelai::parser::expressions::trueexpression) \
	fn(lorelai::parser::expressions::numberexpression) \
	fn(lorelai::parser::expressions::stringexpression) \
	fn(lorelai::parser::expressions::varargexpression) \
	fn(lorelai::parser::expressions::nameexpression)

#define LORELAI_EXPRESSION_CLASS_MACRO(fn) \
	fn(lorelai::parser::expressions::nilexpression) \
	fn(lorelai::parser::expressions::falseexpression) \
	fn(lorelai::parser::expressions::trueexpression) \
	fn(lorelai::parser::expressions::numberexpression) \
	fn(lorelai::parser::expressions::stringexpression) \
	fn(lorelai::parser::expressions::varargexpression) \
	fn(lorelai::parser::expressions::nameexpression) \
	fn(lorelai::parser::expressions::tableexpression) \
	fn(lorelai::parser::expressions::enclosedexpression) \
	fn(lorelai::parser::expressions::binopexpression) \
	fn(lorelai::parser::expressions::unopexpression) \
	fn(lorelai::parser::expressions::indexexpression) \
	fn(lorelai::parser::expressions::dotexpression) \
	fn(lorelai::parser::expressions::functioncallexpression) \
	fn(lorelai::parser::expressions::functionexpression)

#include "types.hpp"
#include "node.hpp"
#include <unordered_map>
#include <vector>

namespace lorelai {
	class lexer;
	namespace parser {
		// ANY expression, containing literals expression unary and binary ops indexing etc.
		class expression {
		public:
			static std::shared_ptr<node> read(lexer &lex, bool postexp = true);
		};

		// lvalue, functioncall or enclosed expression `(` expr `)`
		class prefixexpression : public expression {
		public:
			static std::shared_ptr<node> read(lexer &lex);
		};

		// actually just lvalue
		class varexpression : public prefixexpression {
		public:
			static std::shared_ptr<node> read(std::shared_ptr<node> prefixexp, lexer &lex);
		};

		namespace expressions {
			class nilexpression : public node, public expression {
			public:
				nilexpression() { }
				nilexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};

			class falseexpression : public node, public expression {
			public:
				falseexpression() { }
				falseexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};

			class trueexpression : public node, public expression {
			public:
				trueexpression() { }
				trueexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};

			class numberexpression : public node, public expression {
			public:
				numberexpression(lexer &lex);
				numberexpression(number num) : data(num) { }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

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

				static bool applicable(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
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
				string tostring() override;
			};

			class nameexpression : public node, public varexpression {
			public:
				nameexpression(string data) : name(data) { }
				nameexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			public:
				string name;
			};

			class tableexpression : public branch, public expression {
			public:
				tableexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			public:
				std::vector<std::pair<std::shared_ptr<node>, std::shared_ptr<node>>> tabledata;
			};


			class enclosedexpression : public branch, public prefixexpression {
			public:
				enclosedexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};


			class binopexpression : public branch, public expression {
			public:
				const static std::vector<std::pair<bool, std::vector<string>>> priorities;
				const static std::unordered_map<string, int> prioritymap;

				binopexpression(std::shared_ptr<node> _lhs, string _op, std::shared_ptr<node> _rhs) : lhs(_lhs), op(_op), rhs(_rhs) {
					children.push_back(lhs);
					children.push_back(rhs);
				}

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::shared_ptr<node> lhs, rhs;
				string op;
			};

			class unopexpression : public branch, public expression {
			public:
				unopexpression(string _op, std::shared_ptr<node> _expr) : op(_op), expr(_expr) {
					children.push_back(expr);
				}

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::shared_ptr<node> expr;
				string op;
			};

			class functioncallexpression : public branch, public prefixexpression {
			public:
				functioncallexpression(std::shared_ptr<node> prefixexp, lexer &lex);
				functioncallexpression(lexer &lex) : functioncallexpression(prefixexpression::read(lex), lex) { }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
				static bool applicable(lexer &lex);

			public:
				std::shared_ptr<node> funcexpr;
				std::shared_ptr<node> methodname;
				std::shared_ptr<node> arglist;
			};

			class indexexpression : public branch, public varexpression {
			protected:
				indexexpression() { }
			public:
				indexexpression(std::shared_ptr<node> prefixexp, lexer &lex);
				indexexpression(lexer &lex) : indexexpression(prefixexpression::read(lex), lex) { }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			public:
				std::shared_ptr<node> prefix, index;
			};

			class dotexpression : public indexexpression {
			public:
				dotexpression(std::shared_ptr<node> prefixexp, lexer &lex);
				dotexpression(lexer &lex) : dotexpression(prefixexpression::read(lex), lex) { }
				dotexpression(std::shared_ptr<node> &_prefix, std::shared_ptr<node> &_index) : prefix(_prefix), index(_index) { }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			public:
				std::shared_ptr<node> prefix, index;
			};

			class functionexpression : public branch, public expression {
			public:
				functionexpression(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			public:
				std::shared_ptr<node> body;
			};
	
		}
	}
}

#endif // EXPRESSIONS_HPP_
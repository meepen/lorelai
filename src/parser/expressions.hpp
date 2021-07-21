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
	fn(expressions::nilexpression) \
	fn(expressions::falseexpression) \
	fn(expressions::trueexpression) \
	fn(expressions::numberexpression) \
	fn(expressions::stringexpression) \
	fn(expressions::varargexpression) \
	fn(expressions::nameexpression) \
	fn(expressions::tableexpression) \
	fn(expressions::enclosedexpression) \
	fn(expressions::binopexpression) \
	fn(expressions::unopexpression) \
	fn(expressions::indexexpression) \
	fn(expressions::dotexpression) \
	fn(expressions::functioncallexpression) \
	fn(expressions::functionexpression)

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
			static node *read(lexer &lex, bool postexp = true);
		};

		// lvalue, functioncall or enclosed expression `(` expr `)`
		class prefixexpression : public expression {
		public:
			static node *read(lexer &lex);
		};

		// actually just lvalue
		class varexpression : public prefixexpression {
		public:
			static node *read(node *prefixexp, lexer &lex);
		};

		namespace expressions {
			class nilexpression : public node, public expression {
			public:
				nilexpression() { }
				nilexpression(lexer &lex);

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class falseexpression : public node, public expression {
			public:
				falseexpression() { }
				falseexpression(lexer &lex);

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class trueexpression : public node, public expression {
			public:
				trueexpression() { }
				trueexpression(lexer &lex);

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class numberexpression : public node, public expression {
			public:
				numberexpression(lexer &lex);
				numberexpression(number num) : data(num) { }

				void accept(visitor &visit, node *&container) override;
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

				void accept(visitor &visit, node *&container) override;
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

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class nameexpression : public node, public varexpression {
			public:
				nameexpression(string data) : name(data) { }
				nameexpression(lexer &lex);

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			public:
				string name;
			};

			class tableexpression : public branch, public expression {
			public:
				tableexpression(lexer &lex);
				virtual ~tableexpression() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
				std::vector<node **> getchildren() override {
					std::vector<node **> children;
					for (auto &child : arraypart) {
						children.push_back(&child);
					}

					for (auto &hash : hashpart) {
						children.push_back(&hash.first);
						children.push_back(&hash.second);
					}

					return children;
				}

			public:
				std::vector<node *> arraypart;
				std::vector<std::pair<node *, node *>> hashpart;
			};


			class enclosedexpression : public branch, public prefixexpression {
			public:
				enclosedexpression(lexer &lex);
				enclosedexpression(node *_enclosed) : enclosed(_enclosed) { }
				virtual ~enclosedexpression() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;


				std::vector<node **> getchildren() override {
					return { &enclosed };
				}

			public:
				node *enclosed = nullptr;
			};


			class binopexpression : public branch, public expression {
			public:
				const static std::vector<std::pair<bool, std::vector<string>>> priorities;
				const static std::unordered_map<string, int> prioritymap;

				binopexpression(node *_lhs, string _op, node *_rhs)
					: lhs(_lhs), op(_op), rhs(_rhs) { }
				virtual ~binopexpression() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &lhs, &rhs };
				}

			public:
				node *lhs = nullptr, *rhs = nullptr;
				string op;
			};

			class unopexpression : public branch, public expression {
			public:
				unopexpression(string _op, node *_expr)
					: op(_op), expr(_expr) { }
				virtual ~unopexpression() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &expr };
				}

			public:
				node *expr = nullptr;
				string op;
			};

			class functioncallexpression : public branch, public prefixexpression {
			public:
				functioncallexpression(node *prefixexp, lexer &lex);
				functioncallexpression(lexer &lex) : functioncallexpression(prefixexpression::read(lex), lex) { }
				virtual ~functioncallexpression() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
				static bool applicable(lexer &lex);

				std::vector<node **> getchildren() override {
					return { &funcexpr, &arglist };
				}

			public:
				node *funcexpr = nullptr;
				optional<string> methodname { };
				node *arglist = nullptr;
			};

			class indexexpression : public branch, public varexpression {
			protected:
				indexexpression() { }
			public:
				indexexpression(node *prefixexp, lexer &lex);
				indexexpression(lexer &lex) : indexexpression(prefixexpression::read(lex), lex) { }
				virtual ~indexexpression() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &prefix, &index };
				}

			public:
				node *prefix = nullptr, *index = nullptr;
			};

			class dotexpression : public branch, public varexpression {
			public:
				dotexpression(node *prefixexp, lexer &lex);
				dotexpression(lexer &lex) : dotexpression(prefixexpression::read(lex), lex) { }
				dotexpression(node *&_prefix, string &_index) : prefix(_prefix), index(_index) { }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &prefix };
				}

			public:
				node *prefix = nullptr;
				string index;
			};

			class functionexpression : public branch, public expression {
			public:
				functionexpression(lexer &lex);
				virtual ~functionexpression() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &body };
				}

			public:
				node *body = nullptr;
			};
	
		}
	}
}

#endif // EXPRESSIONS_HPP_
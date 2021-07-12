#ifndef STATEMENTS_HPP_
#define STATEMENTS_HPP_

#define LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(statements::returnstatement) \
	fn(statements::dostatement) \
	fn(statements::whilestatement) \
	fn(statements::repeatstatement) \
	fn(statements::localassignmentstatement) \
	fn(statements::localfunctionstatement) \
	fn(statements::fornumstatement) \
	fn(statements::forinstatement) \
	fn(statements::ifstatement) \
	fn(statements::elseifstatement) \
	fn(statements::elsestatement) \
	fn(statements::functionstatement) \
	fn(statements::functioncallstatement) \
	fn(statements::assignmentstatement)

#define LORELAI_STATEMENT_CLASS_MACRO(fn) \
	LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(statements::breakstatement)

#include <vector>
#include <memory>
#include "types.hpp"
#include "node.hpp"

namespace lorelai {
	class lexer;
	namespace parser {
		class chunk;

		class statement {
		public:
			static node *read(lexer &lex);

			virtual bool isfinal() { return false; }
		};

		class blockstatement : public branch, public statement {
		public:
			std::vector<node **> getchildren() override {
				return { &block };
			}
			virtual ~blockstatement() { destroy(); }

		public:
			node *block = nullptr;
		};

		class loopstatement : public blockstatement {
		public:
			std::vector<node **> getchildren() override {
				return { &conditional, &block };
			}

		public:
			node *conditional = nullptr;
		};

		namespace statements {
			class returnstatement : public branch, public statement {
			public:
				returnstatement(lexer &lex);
				virtual ~returnstatement() { destroy(); }

				bool isfinal() override { return true; }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					std::vector<node **> children;

					for (auto &ret : retlist) {
						children.push_back(&ret);
					}

					return children;
				}

			public:
				std::vector<node *> retlist;
			};
			
			class dostatement : public blockstatement {
			protected:
				dostatement() { }
			public:
				dostatement(lexer &lex);
				virtual ~dostatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class whilestatement : public loopstatement {
			public:
				whilestatement(lexer &lex);
				virtual ~whilestatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class repeatstatement : public loopstatement {
			public:
				repeatstatement(lexer &lex);
				virtual ~repeatstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class localassignmentstatement : public branch, public statement {
			public:
				localassignmentstatement(lexer &lex);
				virtual ~localassignmentstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					std::vector<node **> children;

					for (auto &rt : right) {
						children.push_back(&rt);
					}

					return children;
				}

			public:
				std::vector<string> left;
				std::vector<node *> right;
			};

			class localfunctionstatement : public blockstatement {
			public:
				localfunctionstatement(lexer &lex);
				virtual ~localfunctionstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &name, &block };
				}

			public:
				node *name = nullptr;
			};

			class fornumstatement : public loopstatement {
			public:
				fornumstatement(string name, lexer &lex);
				virtual ~fornumstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					std::vector<node **> children;

					children.push_back(&startexpr);
					children.push_back(&endexpr);
					if (stepexpr) {
						children.push_back(&stepexpr);
					}
					children.push_back(&block);

					return children;
				}

			public:
				string itername;
				node *startexpr = nullptr, *endexpr = nullptr, *stepexpr = nullptr;
			};

			class forinstatement : public loopstatement {
			public:
				forinstatement(string name, lexer &lex);
				virtual ~forinstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					std::vector<node **> children;

					for (auto &inexpr : inexprs) {
						children.push_back(&inexpr);
					}

					children.push_back(&block);

					return children;
				}

			public:
				std::vector<string> iternames;
				std::vector<node *> inexprs;
			};

			class breakstatement : public node, public statement {
			public:
				breakstatement(lexer &lex);

				bool isfinal() override { return true; }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};
			
			class ifstatement : public blockstatement {
			public:
				ifstatement(lexer &lex);
				virtual ~ifstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					std::vector<node **> children;

					children.push_back(&conditional);
					children.push_back(&block);
					for (auto &elseif : elseifs) {
						children.push_back(&elseif);
					}
					if (elseblock) {
						children.push_back(&elseblock);
					}

					return children;
				}

			public:
				node *conditional = nullptr;
				node *elseblock = nullptr;
				std::vector<node *> elseifs;
			};

			class elseifstatement : public blockstatement {
			public:
				elseifstatement(lexer &lex);
				virtual ~elseifstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &conditional, &block };
				}

			public:
				node *conditional = nullptr;
			};

			class elsestatement : public blockstatement {
			public:
				elsestatement(lexer &lex);
				virtual ~elsestatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;
			};

			class functionstatement : public blockstatement {
			public:
				functionstatement(lexer &lex);
				virtual ~functionstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &name, &block };
				}

			public:
				node *name = nullptr;
				optional<string> method;
			};

			class functioncallstatement : public branch, public statement {
			public:
				functioncallstatement(node *functioncall) : callexpr(functioncall) { }
				virtual ~functioncallstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					return { &callexpr };
				}

			public:
				node *callexpr = nullptr;
			};

			class assignmentstatement : public branch, public statement {
			public:
				assignmentstatement(node *exp, lexer &lex);
				virtual ~assignmentstatement() { destroy(); }

				void accept(visitor &visit, node *&container) override;
				string tostring() override;

				std::vector<node **> getchildren() override {
					std::vector<node **> children;

					for (auto &l : left) {
						children.push_back(&l);
					}
					for (auto &r : right) {
						children.push_back(&r);
					}

					return children;
				}

			public:
				std::vector<node *> left;
				std::vector<node *> right;
			};
		}
	}
}

#endif // STATEMENTS_HPP_
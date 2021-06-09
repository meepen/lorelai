#ifndef STATEMENTS_HPP_
#define STATEMENTS_HPP_

#define LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(lorelai::parser::statements::returnstatement) \
	fn(lorelai::parser::statements::dostatement) \
	fn(lorelai::parser::statements::whilestatement) \
	fn(lorelai::parser::statements::repeatstatement) \
	fn(lorelai::parser::statements::localassignmentstatement) \
	fn(lorelai::parser::statements::localfunctionstatement) \
	fn(lorelai::parser::statements::fornumstatement) \
	fn(lorelai::parser::statements::forinstatement) \
	fn(lorelai::parser::statements::ifstatement) \
	fn(lorelai::parser::statements::functionstatement) \
	fn(lorelai::parser::statements::functioncallstatement) \
	fn(lorelai::parser::statements::assignmentstatement)

#define LORELAI_STATEMENT_CLASS_MACRO(fn) \
	LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(lorelai::parser::statements::breakstatement)

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
			static std::shared_ptr<node> read(lexer &lex);

			virtual bool isfinal() { return false; }
		};

		class blockstatement : public branch, public statement {
		public:
			std::shared_ptr<node> block;
		};

		class loopstatement : public blockstatement {
		public:
			std::shared_ptr<node> conditional;
		};

		namespace statements {
			class returnstatement : public branch, public statement {
			public:
				returnstatement(lexer &lex);

				bool isfinal() override { return true; }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};
			
			class dostatement : public blockstatement {
			protected:
				dostatement() { }
			public:
				dostatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class whilestatement : public loopstatement {
			public:
				whilestatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class repeatstatement : public loopstatement {
			public:
				repeatstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class localassignmentstatement : public branch, public statement {
			public:
				localassignmentstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::vector<std::shared_ptr<node>> left;
				std::vector<std::shared_ptr<node>> right;
			};

			class localfunctionstatement : public branch, public statement {
			public:
				localfunctionstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::shared_ptr<node> name;
				std::shared_ptr<node> body;
			};

			class fornumstatement : public loopstatement {
			public:
				fornumstatement(std::shared_ptr<node> name, lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::shared_ptr<node> itername;
				std::shared_ptr<node> startexpr, endexpr, stepexpr;
			};

			class forinstatement : public loopstatement {
			public:
				forinstatement(std::shared_ptr<node> name, lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::vector<std::shared_ptr<node>> iternames;
				std::shared_ptr<node> inexpr;
			};

			class breakstatement : public node, public statement {
			public:
				breakstatement(lexer &lex);

				bool isfinal() override { return true; }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};
			
			class ifstatement : public branch, public statement {
			public:
				ifstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::vector<std::shared_ptr<node>> conditionals;
				std::vector<std::shared_ptr<node>> blocks;
			};

			class functionstatement : public statement, public branch {
			public:
				functionstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::vector<std::shared_ptr<node>> names;
				std::shared_ptr<node> method;
				std::shared_ptr<node> body;
			};

			class functioncallstatement : public branch, public statement {
			public:
				functioncallstatement(std::shared_ptr<node> functioncall) {
					children.push_back(functioncall);
				}

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class assignmentstatement : public branch, public statement {
			public:
				assignmentstatement(std::shared_ptr<node> exp, lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::vector<std::shared_ptr<node>> left;
				std::vector<std::shared_ptr<node>> right;
			};
		}
	}
}

#endif // STATEMENTS_HPP_
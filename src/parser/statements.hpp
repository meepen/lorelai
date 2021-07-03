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
				string tostring() override;
			};
			
			class dostatement : public blockstatement {
			protected:
				dostatement() { }
			public:
				dostatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};

			class whilestatement : public loopstatement {
			public:
				whilestatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};

			class repeatstatement : public loopstatement {
			public:
				repeatstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};

			class localassignmentstatement : public branch, public statement {
			public:
				localassignmentstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::vector<string> left;
				std::vector<std::shared_ptr<node>> right;
			};

			class localfunctionstatement : public branch, public statement {
			public:
				localfunctionstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::shared_ptr<node> name;
				std::shared_ptr<node> body;
			};

			class fornumstatement : public loopstatement {
			public:
				fornumstatement(string name, lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				string itername;
				std::shared_ptr<node> startexpr, endexpr, stepexpr;
			};

			class forinstatement : public loopstatement {
			public:
				forinstatement(string name, lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::vector<string> iternames;
				std::vector<std::shared_ptr<node>> inexprs;
			};

			class breakstatement : public node, public statement {
			public:
				breakstatement(lexer &lex);

				bool isfinal() override { return true; }

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};
			
			class ifstatement : public branch, public statement {
			public:
				ifstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::shared_ptr<node> conditional;
				std::shared_ptr<node> block;
				std::shared_ptr<node> elseblock;
				std::vector<std::shared_ptr<node>> elseifs;
			};

			class elseifstatement : public branch, public statement {
			public:
				elseifstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::shared_ptr<node> conditional;
				std::shared_ptr<node> block;
			};

			class elsestatement : public branch, public statement {
			public:
				elsestatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::shared_ptr<node> block;
			};

			class functionstatement : public statement, public branch {
			public:
				functionstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::shared_ptr<node> name;
				optional<string> method;
				std::shared_ptr<node> body;
			};

			class functioncallstatement : public branch, public statement {
			public:
				functioncallstatement(std::shared_ptr<node> functioncall) {
					children.push_back(functioncall);
				}

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;
			};

			class assignmentstatement : public branch, public statement {
			public:
				assignmentstatement(std::shared_ptr<node> exp, lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
				string tostring() override;

			public:
				std::vector<std::shared_ptr<node>> left;
				std::vector<std::shared_ptr<node>> right;
			};
		}
	}
}

#endif // STATEMENTS_HPP_
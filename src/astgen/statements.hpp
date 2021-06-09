#ifndef STATEMENTS_HPP_
#define STATEMENTS_HPP_

#define LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(lorelai::astgen::statements::returnstatement) \
	fn(lorelai::astgen::statements::dostatement) \
	fn(lorelai::astgen::statements::whilestatement) \
	fn(lorelai::astgen::statements::repeatstatement) \
	fn(lorelai::astgen::statements::localsstatement) \
	fn(lorelai::astgen::statements::localfunctionstatement) \
	fn(lorelai::astgen::statements::fornumstatement) \
	fn(lorelai::astgen::statements::forinstatement) \
	fn(lorelai::astgen::statements::ifstatement) \
	fn(lorelai::astgen::statements::functionstatement)

#define LORELAI_STATEMENT_CLASS_MACRO(fn) \
	LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(lorelai::astgen::statements::breakstatement)

#include <vector>
#include <memory>
#include "types.hpp"
#include "node.hpp"

namespace lorelai {
	class lexer;
	namespace astgen {
		class chunk;

		class statement {
		public:
			static std::shared_ptr<node> read(lexer &lex);
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

			class localsstatement : public branch, public statement {
			public:
				localsstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::vector<std::shared_ptr<node>> names;
				std::vector<std::shared_ptr<node>> initializers;
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

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};
			
			class ifstatement : public blockstatement {
			public:
				ifstatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;

			public:
				std::shared_ptr<node> conditional;
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
		}
	}
}

#endif // STATEMENTS_HPP_
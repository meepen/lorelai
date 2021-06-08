#ifndef STATEMENTS_HPP_
#define STATEMENTS_HPP_

#define LORELAI_STATEMENT_CLASS_MACRO(fn) \
	fn(lorelai::astgen::statements::returnstatement) \
	fn(lorelai::astgen::statements::dostatement) \
	fn(lorelai::astgen::statements::whilestatement) \
	fn(lorelai::astgen::statements::repeatstatement) \
	fn(lorelai::astgen::statements::localsstatement)

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
			std::shared_ptr<node> conditional;
			std::shared_ptr<node> block;
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

			class whilestatement : public blockstatement {
			public:
				whilestatement(lexer &lex);

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class repeatstatement : public blockstatement {
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
		}
	}
}

#endif // STATEMENTS_HPP_
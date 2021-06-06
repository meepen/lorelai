#ifndef STATEMENTS_HPP_
#define STATEMENTS_HPP_

#define LORELAI_STATEMENT_CLASS_MACRO(fn) \
	fn(lorelai::astgen::statements::returnstatement) \
	fn(lorelai::astgen::statements::dostatement) \
	fn(lorelai::astgen::statements::whilestatement)

#include <vector>
#include <memory>
#include "lexer.hpp"
#include "types.hpp"
#include "node.hpp"
#include "expressions.hpp"
#include "errors.hpp"

namespace lorelai {
	namespace astgen {
		class statement {
		public:
			static std::shared_ptr<node> read(lexer &lex);
		};

		class blockstatement : public branch, public statement {
		public:
			std::shared_ptr<node> conditional;
			std::vector<std::shared_ptr<node>> statements;
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
				dostatement(lexer &lex) {
					// consume do
					lex.read();

					// try to read statement list
					while (lex.lookahead() && lex.lookahead().value() != "end") {
						auto stmt = statement::read(lex);
						if (!stmt) {
							throw error::expected_for("statement", "do .. end", lex.lookahead().value_or("no value"));
						}

						statements.push_back(stmt);
						children.push_back(stmt);
					}

					auto ensure_end = lex.read();

					if (ensure_end != "end") {
						throw error::expected_for("end", "do .. end", lex.lookahead().value_or("no value"));
					}
				}

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};

			class whilestatement : public blockstatement {
			public:
				whilestatement(lexer &lex) {
					// consume while
					lex.read();
					conditional = expression::read(lex);
					children.push_back(conditional);

					auto word = lex.read();
					if (word != "do") {
						throw error::expected_for("do", "while .. do .. end", word);
					}

					// try to read statement list
					while (lex.lookahead() && lex.lookahead().value() != "end") {
						auto stmt = statement::read(lex);
						if (!stmt) {
							throw error::expected_for("statement", "while .. do .. end", lex.lookahead().value_or("no value"));
						}

						statements.push_back(stmt);
						children.push_back(stmt);
					}

					word = lex.read();
					if (word != "end") {
						throw error::expected_for("end", "while .. do .. end", word);
					}
				}

				bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			};
		}
	}
}

#endif // STATEMENTS_HPP_
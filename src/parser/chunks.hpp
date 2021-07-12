#ifndef CHUNKS_HPP_
#define CHUNKS_HPP_

#include "node.hpp"
#include "types.hpp"
#include <vector>

namespace lorelai {
	class lexer;
	namespace parser {
		class chunk : public branch {
		private:
			void initialize(lexer &lex);

		public:
			chunk(lexer &lex);
			chunk(string data, bool expect_eof = false);
			virtual ~chunk() { destroy(); }

			void accept(visitor &visit, node *&container) override;
			string tostring() override { return ""; }
			
			std::vector<node **> getchildren() override {
				std::vector<node **> children;

				for (auto &child : childstatements) {
					children.push_back(&child);
				}

				return children;
			}


		public:
			std::vector<node *> childstatements;
		};
	}
}

#endif // CHUNKS_HPP_
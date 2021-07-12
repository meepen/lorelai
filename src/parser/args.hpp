#ifndef ARGS_HPP_
#define ARGS_HPP_

#include "node.hpp"
#include <memory>

namespace lorelai {
	class lexer;

	namespace parser {
		class args : public branch {
		public:
			args(lexer &lex);
			virtual ~args() { destroy(); }

			static bool applicable(lexer &lex);
			
			void accept(visitor &visit, node *&container) override;
			string tostring() override;

			std::vector<node **> getchildren() override {
				std::vector<node **> children;
				for (auto &arg : arglist) {
					children.push_back(&arg);
				}

				return children;
			}

		public:
			std::vector<node *> arglist;
		};
	}
}

#endif // ARGS_HPP_
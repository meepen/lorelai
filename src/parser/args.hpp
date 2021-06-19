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

			static bool applicable(lexer &lex);
			
			bool accept(visitor &visit, std::shared_ptr<node> &container, void *data) override;
		};
	}
}

#endif // ARGS_HPP_
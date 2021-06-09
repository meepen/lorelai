#ifndef FUNCBODY_HPP_
#define FUNCBODY_HPP_

#include "node.hpp"
#include <vector>
#include <memory>

namespace lorelai {
	class lexer;
	namespace parser {
		class visitor;
		class funcbody : public branch {
		public:
			funcbody(lexer &lex);

			bool accept(visitor &visit, std::shared_ptr<node> &container) override;

		public:
			std::vector<std::shared_ptr<node>> params;
			std::shared_ptr<node> block;
		};
	}
}

#endif // FUNCBODY_HPP_
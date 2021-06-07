#ifndef CHUNKS_HPP_
#define CHUNKS_HPP_

#include "node.hpp"
#include "types.hpp"

namespace lorelai {
	class lexer;
	namespace astgen {
		class chunk : public branch {
		protected:
			chunk() { }

		public:
			chunk(lexer &lex);
			chunk(string data);

			bool accept(visitor &visit, std::shared_ptr<node> &container) override;
		};
	}
}

#endif // CHUNKS_HPP_
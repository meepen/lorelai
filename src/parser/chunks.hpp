#ifndef CHUNKS_HPP_
#define CHUNKS_HPP_

#include "node.hpp"
#include "types.hpp"

namespace lorelai {
	class lexer;
	namespace parser {
		class chunk : public branch {
		private:
			void initialize(lexer &lex);

		public:
			chunk(lexer &lex);
			chunk(string data, bool expect_eof = false);

			bool accept(visitor &visit, std::shared_ptr<node> &container) override;
			string tostring() override { return ""; }
		};
	}
}

#endif // CHUNKS_HPP_
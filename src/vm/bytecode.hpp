#ifndef BYTECODE_HPP_
#define BYTECODE_HPP_

#include "bytecode/prototype.hpp"
#include "parser.hpp"

namespace lorelai {
	namespace bytecode {
		prototype create(parser::chunk &chunk);

		static prototype create(string code, bool expect_eof = true) {
			parser::chunk mainchunk(code, expect_eof);
			return create(mainchunk);
		}
	}
}

#endif // BYTECODE_HPP_
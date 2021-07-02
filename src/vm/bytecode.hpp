#ifndef BYTECODE_HPP_
#define BYTECODE_HPP_

#include "proto/bytecode.pb.h"
#include "parser.hpp"

namespace lorelai {
	namespace bytecode {
		std::shared_ptr<prototype> create(parser::chunk &chunk);

		static std::shared_ptr<prototype> create(string code, bool expect_eof = true) {
			parser::chunk mainchunk(code, expect_eof);
			return create(mainchunk);
		}
	}
}

#endif // BYTECODE_HPP_
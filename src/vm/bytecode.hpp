#ifndef BYTECODE_HPP_
#define BYTECODE_HPP_

#include "proto/bytecode.pb.h"
#include "parser.hpp"

namespace lorelai {
	namespace vm {
		std::shared_ptr<bytecode::prototype> parse(parser::chunk &chunk);

		static std::shared_ptr<bytecode::prototype> parse(string code, bool expect_eof = true) {
			parser::chunk mainchunk(code, expect_eof);
			return parse(mainchunk);
		}
	}
}

#endif // BYTECODE_HPP_
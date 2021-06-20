#ifndef BYTECODE_HPP_
#define BYTECODE_HPP_

#include "proto/bytecode.pb.h"
#include "parser.hpp"

namespace lorelai {
	namespace vm {
		bytecode::prototype parse(parser::chunk &chunk);

		static bytecode::prototype parse(string code, bool expect_eof = true) {
			parser::chunk mainchunk(code, expect_eof);
			return parse(mainchunk);
		}

		namespace binaryoperators {
			enum binaryoperator {
				ADD,
				SUBTRACT,
				DIVIDE,
				MULTIPLY,
				POWER,
				MODULO,
				CONCAT,
				INDEX,
				OR,
				AND,

				LESSTHAN,
				LESSTHANEQUAL,
				GREATERTHAN,
				GREATERTHANEQUAL,
				EQUALS,
				NOTEQUALS
			};
		}

		namespace unaryoperators {
			enum unaryoperator {
				NOT,
				MINUS,
				LENGTH
			};
		}
	}
}

#endif // BYTECODE_HPP_
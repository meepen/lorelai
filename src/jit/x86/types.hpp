#ifndef JIT_TYPES_HPP_
#define JIT_TYPES_HPP_

#include <cstdint>
#include <memory>

namespace lorelai {
	namespace jit {
		class type {
			struct stackdata;
			using binopfunc = void (*)(std::uintptr_t op, stackdata *out, stackdata *const lhs, stackdata *const rhs);
			using unopfunc = void (*)(std::uintptr_t op, stackdata *out, stackdata *const data);
			// used in jit
			struct stackdata {
				std::uint8_t datatype;
				std::uint8_t unused[3];

				// this will have to be in a metatable eventually
				binopfunc binop;
				unopfunc unop;

				std::shared_ptr<type> cdata;
			};

		};

		namespace types {
			class number : public type {

			};
		}
	}
}

#endif // JIT_TYPES_HPP_
#ifndef JIT_TYPES_HPP_
#define JIT_TYPES_HPP_

#include <cstdint>
#include <memory>

#include "state.hpp"

namespace lorelai {
	namespace jit {
		struct object;
		using luafunction = void (*)(object *out, object *const in);


#pragma pack(push, 1)
		struct typeinstructions {
			object *(*metatable)(object *obj);
			void (*binaryop)(object *out, std::uintptr_t op, object *const lhs, object *const rhs) = nullptr;
			void (*unaryop)(object *out, std::uintptr_t op, object *const obj) = nullptr;
			void (*index)(object *out, object *const obj, object *const index) = nullptr;
			void (*newindex)(object *out, object *const obj, object *const index, object *const value) = nullptr;
		};

		typeinstructions default_instructions;

		struct object {
			typeinstructions *instructions = &default_instructions;

			
			void *data(state &state) {
				return state.
				
			}
		};

		struct stringobject : public object {
			static typeinstructions stringtype;

			std::string data;
		}
#pragma pack(pop)
	}
}

#endif // JIT_TYPES_HPP_
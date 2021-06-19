#ifndef JIT_TYPES_HPP_
#define JIT_TYPES_HPP_

#include <cstdint>
#include <memory>

#include "state.hpp"
#include "types.hpp"

namespace lorelai {
	namespace vm {
		struct object;
		using luafunction = void (*)(object *out, object *const in);

		struct typeinstructions {
			static void noop(object *) { }

			const char *type = nullptr;
			void (*free)(object *) = noop;
			object *(*add)(object *lhs, object *rhs) = nullptr;
			object *(*sub)(object *lhs, object *rhs) = nullptr;
			object *(*div)(object *lhs, object *rhs) = nullptr;
			object *(*mul)(object *lhs, object *rhs) = nullptr;
			object *(*pow)(object *lhs, object *rhs) = nullptr;
			object *(*mod)(object *lhs, object *rhs) = nullptr;
			object *(*lessthan)(object *lhs, object *rhs) = nullptr;
			object *(*greaterthan)(object *lhs, object *rhs) = nullptr;
			object *(*concat)(object *lhs, object *rhs) = nullptr;
			object *(*index)(object *lhs, object *rhs) = nullptr;
			object *(*call)(object *obj) = nullptr;
			object *(*len)(object *obj) = nullptr;
		};

		struct object {
			static typeinstructions niltype;

			typeinstructions *instructions = &niltype;
			object *metatable = nullptr;
		};

		struct numberobject : public object {
			static typeinstructions numbertype;

			numberobject() {
				instructions = &numbertype;
			}
			numberobject(number num) {
				instructions = &numbertype;
				data = num;
			}

			number data = 0;
		};
	}
}

#endif // JIT_TYPES_HPP_
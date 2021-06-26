#ifndef JIT_TYPES_HPP_
#define JIT_TYPES_HPP_

#include <cstdint>
#include <memory>

#include "state.hpp"
#include "types.hpp"
#include "bytecode.hpp"
#include <iostream>

#define LORELAI_SOFTWARE_DEFAULT_FUNCTION(args...) (args) { except(); }

namespace lorelai {
	namespace vm {
		struct object;
		using luafunction = void (*)(object *out, object *const in);

		class object {
		protected:
			void except() { throw; }

		public:
			virtual const char *type() const { return "nil"; }

			virtual void add         LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void sub         LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void div         LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void mul         LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void pow         LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void mod         LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void lessthan    LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void greaterthan LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void concat      LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual void index       LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *lhs, const object *rhs);
			virtual size_t call      LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, size_t nrets, size_t nargs);
			virtual void len         LORELAI_SOFTWARE_DEFAULT_FUNCTION(object **out, const object *obj);

			object *metatable = nullptr;
		};

		class numberobject : public object {
		public:
			numberobject(number num) : data(num) { }
		public:
			const char *type() const override { return "number"; }


		public:
			number data = 0;
		};

		class functionobject : public object {
		public:
			functionobject(const bytecode::prototype &proto) : data(proto) { }
		public:
			const char *type() const override { return "function"; }
			size_t call(object **stack, size_t nrets, size_t nargs) override {
			}

		public:
			bytecode::prototype data;
		};
	}
}

#endif // JIT_TYPES_HPP_
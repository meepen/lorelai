#ifndef STATE_HPP_
#define STATE_HPP_

#include <string>
#include <memory>
#include "types.hpp"

#define LORELAI_TYPES(fn) \
	fn(NIL) \
	fn(BOOL) \
	fn(NUMBER) \
	fn(TABLE) \
	fn(STRING) \
	fn(LUAFUNCTION) \
	fn(CFUNCTION)

namespace lorelai {
	namespace bytecode {
		class prototype;
	}

	namespace vm {
#define LORELAI_LITERAL(x) x,
		enum _type {
			LORELAI_TYPES(LORELAI_LITERAL)
		};
#undef LORELAI_LITERAL
#define LORELAI_STRING(x) #x,
		static const char *typenames[] = {
			LORELAI_TYPES(LORELAI_STRING)
		};
#undef LORELAI_STRING

		class state {
		public:
			class _retdata {
			public:
				_retdata() { }
				_retdata(int base, int size) : retbase(base), retsize(size) { }
				_retdata(const _retdata &r) : retbase(r.retbase), retsize(r.retsize) { }

			public:
				int retbase;
				int retsize;
			};

		public:
			virtual ~state() { }
			virtual const char *backend() const = 0;
			virtual void loadfunction(const bytecode::prototype &code) = 0;
			virtual void loadnumber(number num) = 0;
			virtual void loadstring(string str) = 0;

			void loadfunction(const std::string &code);

			virtual int call(int nargs, int nrets) = 0;


			static std::shared_ptr<state> create();
			static std::shared_ptr<state> createfastest();
		};
	}
}

#endif // STATE_HPP_
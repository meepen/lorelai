#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"
#include "types.hpp"
#include "object.hpp"
#include "gc/gc.hpp"
#include <vector>
#include <unordered_map>

namespace lorelai {
	namespace vm {
		class object;
		class referenceobject;
		class boolobject;
		class nilobject;
		class numberobject;
		class stringobject;
		class tableobject;
		class luafunctionobject;
		class cfunctionobject;

		class softwarestate : public state {
		public:
			template <int size>
			class _stack {
			private:
				_stack(const _stack &other) { }
				_stack(_stack &&other) { }

			public:
				_stack(softwarestate *_st) : st(_st), stackbase(new object[size]) {
					stackptr = stackbase;
					stacktop = stackbase;
					for (int i = 0; i < size; i++) {
						stackbase[i].unset();
					}
				}
				~_stack() {
					delete[] stackbase;
				}

				object &operator[](const int &index) {
					return stackptr[index];
				}

				int pushstack(int reserved) {
					int old = stackptr - stackbase;
					stackptr = stacktop;
					stacktop = stackptr + reserved;
					return old;
				}

				int popstack(const int &old, const state::_retdata &retdata) {
					for (int i = 0; i < retdata.retsize; i++) {
						stackptr[i].set(stackptr[i + retdata.retbase]);
					}

					stacktop = stackptr;
					stackptr = stackbase + old;

					// TODO: nil old stack

					return retdata.retsize;
				}

			public:
				softwarestate *st = nullptr;

				object *const stackbase = nullptr;
				object *stackptr = nullptr;
				object *stacktop = nullptr;
			};
		public:
			softwarestate();
			virtual ~softwarestate() override;

			void initlibs();

			const char *backend() const override { return "software"; }
			void loadfunction(const bytecode::prototype &code) override;
			void loadnumber(number num) override;
			void loadstring(string num) override;
			int call(int nargs, int nrets) override;

			object &operator[](int index) {
				return stack[index];
			}
			_stack<256> *operator->() {
				return &stack;
			}

			static std::shared_ptr<state> create();

		public:
			object registry;

		public:
			object boolean_metatable;
			object nil_metatable;
			object string_metatable;
			object function_metatable;
			object number_metatable;

		public:
			gc::manager memory;

			std::unordered_map<string, stringobject *> stringmap;

		public:
			_stack<256> stack;
		};
	}
}

#endif // SOFTWARE_HPP_
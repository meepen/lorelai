#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"
#include "container.hpp"
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
				struct stackpos {
					int base;
					int top;
				};

			private:
				_stack(const _stack &other) { }
				_stack(_stack &&other) { }

			public:
				_stack(softwarestate *_st) : st(_st), data(new object[size]) {
					for (int i = 0; i < size; i++) {
						data[i].unset();
					}
				}
				~_stack() {
					delete[] data;
				}

				object &operator[](const int index) {
					return data[base + index];
				}

				stackpos pushpointer(int _base) {
					stackpos old = {base, top};
					base = _base;
					top = 0;
					return old;
				}

				int poppointer(const stackpos old, const state::_retdata retdata, int to = 0, int amount = 0) {
					if (amount == -1) {
						amount = retdata.retsize;
					}

					for (int i = 1; i <= std::min(amount, retdata.retsize); i++) {
						data[to + i - 1] = data[retdata.retbase + i - 1];
					}

					for (int i = std::min(amount, retdata.retsize) + 1; i <= amount; i++) {
						data[to + i - 1] = object();
					}

					top = old.top;
					base = old.base;

					return amount;
				}

			public:
				softwarestate *st = nullptr;

				object *data;
				int base = 0;
				int top = 0;
			};
		public:
			softwarestate();
			virtual ~softwarestate() override;

			void initlibs();

			const char *backend() const override { return "software"; }
			void loadfunction(std::shared_ptr<bytecode::prototype> code) override;
			void loadnumber(number num) override;
			_retdata call(int nargs, int nrets) override;

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
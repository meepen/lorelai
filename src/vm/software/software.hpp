#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"
#include "container.hpp"
#include "types.hpp"
#include "object.hpp"
#include <vector>

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
			class _stack {
				struct stackpos {
					int base;
					int top;
				};
			public:
				_stack(softwarestate *_st, int size) : st(_st), data(size) {
					initstack();
				}
				void initstack();

				object &operator[](const int index) {
					if (index >= 0) {
						return data[base + index];
					}
					else {
						return data[base + top + index];
					}
				}

				stackpos pushpointer(int _base) {
					stackpos old = {base, top};
					base = _base;
					top = 0;
					return old;
				}

				int poppointer(const stackpos old, const state::_retdata retdata, int to = 0, int amount = 0);

			public:
				softwarestate *st = nullptr;

				std::vector<object> data;
				int base = 0;
				int top = 0;
			};
		public:
			softwarestate();
			virtual ~softwarestate() override { }

			void initlibs();

			const char *backend() const override { return "software"; }
			void loadfunction(std::shared_ptr<bytecode::prototype> code) override;
			void loadnumber(number num) override;
			_retdata call(int nargs, int nrets) override;

			object &operator[](int index) {
				return stack[index];
			}
			_stack *operator->() {
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
			software::allocator<tableobject> tableallocator;
			software::allocator<luafunctionobject> luafunctionallocator;
			software::allocator<cfunctionobject> cfunctionallocator;

		public:
			_stack stack = _stack(this, 256);
		};
	}
}

#endif // SOFTWARE_HPP_
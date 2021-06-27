#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"
#include "container.hpp"
#include "types.hpp"
#include <vector>

namespace lorelai {
	namespace vm {
		class object;
		using objectcontainer = software::container<object>;
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

				objectcontainer &operator[](const int index) {
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

				std::vector<objectcontainer> data;
				int base = 0;
				int top = 0;
			};
		public:
			softwarestate();
			virtual ~softwarestate() override { }

			void initlibs();
			void initallocators();

			const char *backend() const override { return "software"; }
			void loadfunction(std::shared_ptr<bytecode::prototype> code) override;
			void loadnumber(number num) override;
			_retdata call(int nargs, int nrets) override;

			objectcontainer &operator[](int index) {
				return stack[index];
			}
			_stack *operator->() {
				return &stack;
			}

			static std::shared_ptr<state> create();

		public:
			objectcontainer registry;

		public:
			objectcontainer boolean_metatable = nullptr;
			objectcontainer nil_metatable = nullptr;
			objectcontainer string_metatable = nullptr;
			objectcontainer function_metatable = nullptr;
			objectcontainer number_metatable = nullptr;

		public:
			software::allocator<boolobject, 2> boolallocator;
			software::allocator<nilobject, 1> nilallocator;
			software::allocator<numberobject> numberallocator;
			software::allocator<stringobject> stringallocator;
			software::allocator<tableobject> tableallocator;
			software::allocator<luafunctionobject> luafunctionallocator;
			software::allocator<cfunctionobject> cfunctionallocator;

		public:
			objectcontainer nil;
			objectcontainer trueobj;
			objectcontainer falseobj;
		
		public:
			_stack stack = _stack(this, 256);
		};
	}
}

#endif // SOFTWARE_HPP_
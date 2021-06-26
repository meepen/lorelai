#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"
#include "object.hpp"

namespace lorelai {
	namespace vm {
		class object;

		class softwarestate : public state {
		public:
			template <size_t size>
			class _stack {
				struct stackpos {
					int base;
					int top;
				};
			public:
				_stack(state *_st) : st(_st) {
					for (size_t i = 0; i < sizeof(data) / sizeof(*data); i++) {
						data[i] = std::make_shared<nilobject>();
					}
				}
				~_stack() {
					for (size_t i = 0; i < sizeof(data) / sizeof(*data); i++) {
						data[i] = nullptr;
					}
				}

				std::shared_ptr<object> &operator[](const int index) {
					if (index >= 0) {
						return data[base + index];
					}
					else {
						return data[base + top + index];
					}
				}

				std::shared_ptr<object> &at(const int index) {
					return data[base + index];
				}

				stackpos pushpointer(int _base) {
					stackpos old = {base, top};
					base = _base;
					top = 0;
					return old;
				}

				int poppointer(const stackpos old, const state::_retdata retdata) {
					for (int i = 1; i <= retdata.retsize; i++) {
						data[old.base + old.top + i - 1] = data[retdata.retbase + i - 1];
					}

					top = old.top + retdata.retsize;
					base = old.base;

					return retdata.retsize;
				}

			public:
				state *st = nullptr;

				std::shared_ptr<object> data[size];
				int base = 0;
				int top = 0;
			};

			using stacktype = _stack<256>;
		public:
			softwarestate() {
				initlibs();
			}
			virtual ~softwarestate() override { }

			void initlibs();

			const char *backend() const override { return "software"; }
			void loadfunction(std::shared_ptr<bytecode::prototype> code) override {
				stack[stack.top++] = std::make_shared<luafunctionobject>(code);
			}

			// for now we are just putting these in here to test the api
			void loadnumber(number num) {
				stack[stack.top++] = std::make_shared<numberobject>(num);
			}

			_retdata call(int nargs, int nrets) override;

			std::shared_ptr<object> &operator[](int index) {
				return stack[index];
			}
			stacktype *operator->() {
				return &stack;
			}

			static std::shared_ptr<state> create();

		public:
			std::shared_ptr<tableobject> registry = std::make_shared<tableobject>();
		
		public:
			stacktype stack = stacktype(this);
		};
	}
}

#endif // SOFTWARE_HPP_
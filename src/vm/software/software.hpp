#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"
#include "object.hpp"

namespace lorelai {
	namespace vm {
		class softwarestate : public state {
		public:
			softwarestate() {
				for (size_t i = 0; i < sizeof(stack) / sizeof(*stack); i++) {
					stack[i] = new object();
				}
			}
			~softwarestate() {
				for (size_t i = 0; i < sizeof(stack) / sizeof(*stack); i++) {
					delete stack[i];
				}
			}

			const char *backend() const override { return "software"; }
			void loadfunction(const bytecode::prototype &code) override {
				set(incrtop(), new functionobject(code));
			}

			// for now we are just putting these in here to test the api
			void loadnumber(number num) {
				set(incrtop(), new numberobject(num));
			}

			size_t call(size_t nargs, size_t nrets) override {
				assert(nargs + 1 >= top);

				auto &tocall = stack[top - nargs - 1];
				tocall->call(&tocall, nrets, nargs);
			}

			static std::shared_ptr<state> create();
		
		private:
			object *stack[256];

			void set(int where, object *what) {
				delete stack[where];
				stack[where] = what;
			}

			int top = 0;

			int incrtop() {
				auto ret = top++;
				if (top > sizeof(stack) / sizeof(*stack)) {
					throw;
				}

				return ret;
			}
		};
	}
}

#endif // SOFTWARE_HPP_
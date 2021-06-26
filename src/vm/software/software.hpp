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
					stack[i] = std::make_shared<nilobject>();
				}

				initlibs();
			}

			void initlibs();

			const char *backend() const override { return "software"; }
			void loadfunction(std::shared_ptr<bytecode::prototype> code) override {
				set(incrtop(), std::make_shared<luafunctionobject>(code));
			}

			// for now we are just putting these in here to test the api
			void loadnumber(number num) {
				set(incrtop(), std::make_shared<numberobject>(num));
			}

			size_t call(size_t nargs, size_t nrets) override {
				assert(nargs + 1 >= top);

				auto tocall = stack[top - nargs - 1];
				tocall->call(*this, &stack[top - nargs - 1], nrets, nargs);
			}

			static std::shared_ptr<state> create();

		public:
			std::shared_ptr<tableobject> registry = std::make_shared<tableobject>();
		
		private:
			std::shared_ptr<object> stack[256];

			void set(int where, std::shared_ptr<object> what) {
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
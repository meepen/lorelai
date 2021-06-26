#ifndef STATE_HPP_
#define STATE_HPP_

#include <string>
#include <memory>

namespace lorelai {
	namespace vm {
		namespace bytecode {
			class prototype;
		}

		class state {
		public:
			virtual const char *backend() const = 0;
			virtual void loadfunction(const bytecode::prototype &proto) = 0;
			void loadfunction(const std::string &code);

			virtual size_t call(size_t nargs, size_t nrets) = 0;


			static std::shared_ptr<state> create();
			static std::shared_ptr<state> createfastest();
		};
	}
}

#endif // STATE_HPP_
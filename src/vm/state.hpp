#ifndef STATE_HPP_
#define STATE_HPP_

#include <string>
#include <memory>

namespace lorelai {
	namespace vm {
		class state {
		public:
			virtual const char *backend() const = 0;
			virtual void loadstring(const std::string &code) = 0;

			static std::shared_ptr<state> create();
			static std::shared_ptr<state> createfastest();
		};
	}
}

#endif // STATE_HPP_
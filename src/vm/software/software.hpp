#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"

namespace lorelai {
	namespace vm {
		class softwarestate : public state {
		public:
			const char *backend() const override { return "software"; }
			void loadstring(const std::string &code) override;

			static std::shared_ptr<state> create();
		};
	}
}

#endif // SOFTWARE_HPP_
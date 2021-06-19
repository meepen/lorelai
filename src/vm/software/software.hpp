#ifndef SOFTWARE_HPP_
#define SOFTWARE_HPP_

#include "state.hpp"

namespace lorelai {
	namespace vm {
		class softwarestate : public state {
		public:
			const char *backend() override { return "software"; }
		};
	}
}

#endif // SOFTWARE_HPP_
#ifndef SCOPE_HPP_
#define SCOPE_HPP_

#include "types/types.hpp"

namespace lorelai {
	namespace astgen {

		class scope {
		public:
			scope(std::shared_ptr<scope> parent, bool is_variadic = false) : parent_scope(parent), variadic(is_variadic) {

			}

		public:
			std::vector<string> variable_names = std::vector<string>();
			std::shared_ptr<scope> parent_scope = nullptr;
			bool variadic = false;
		};
	}
}

#endif // SCOPE_HPP_
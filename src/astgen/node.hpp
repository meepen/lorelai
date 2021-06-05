#ifndef BRANCHES_HPP_
#define BRANCHES_HPP_

#include <vector>
#include <memory>
#include "scope.hpp"

namespace lorelai {
	namespace astgen {
		class visitor;
		class node {
		public:
			virtual bool accept(visitor &visit, std::shared_ptr<node> &container) = 0;
		};

		class branch : public node {
		public:
			branch() : children() { }
		public:
			std::vector<std::shared_ptr<node>> children;
			std::shared_ptr<scope> current_scope = nullptr;
		};
	}
}

#endif // BRANCHES_HPP_
#ifndef BRANCHES_HPP_
#define BRANCHES_HPP_

#include <vector>
#include <memory>

namespace lorelai {
	namespace astgen {
		class visitor;
		class node {
		public:
			virtual bool accept(visitor &visit, std::shared_ptr<node> &container) = 0;
		};

		class branch : public node {
		protected:
			void visitchildren(visitor &visit);

		public:
			branch() : children() { }
		public:
			std::vector<std::shared_ptr<node>> children;
		};
	}
}

#endif // BRANCHES_HPP_
#ifndef NODE_HPP_
#define NODE_HPP_

#include <vector>
#include <memory>

namespace lorelai {
	namespace parser {
		class visitor;
		class node {
		public:
			virtual bool accept(visitor &visit, std::shared_ptr<node> &container, void *data) = 0;
		};

		class branch : public node {
		protected:
			void visitchildren(visitor &visit, void *data);

		public:
			branch() : children() { }
		public:
			std::vector<std::shared_ptr<node>> children;
		};
	}
}

#endif // NODE_HPP_
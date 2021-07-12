#ifndef NODE_HPP_
#define NODE_HPP_

#include <vector>
#include <memory>
#include <string>
#include "types.hpp"

namespace lorelai {
	namespace parser {
		class visitor;
		class node {
		public:
			virtual ~node() { }
		public:
			virtual void accept(visitor &visit, node *&container) = 0;
			virtual string tostring() = 0;
		};

		class branch : public node {
		protected:
			void visitchildren(visitor &visit) {
				for (auto child : getchildren()) {
					(*child)->accept(visit, *child);
				}
			}

			void destroy() {
				for (auto child : getchildren()) {
					delete *child;
					*child = nullptr;
				}
			}

		public:
			virtual ~branch() { }

		public:
			virtual std::vector<node **> getchildren() = 0;
		};
	}
}

#endif // NODE_HPP_
#include "node.hpp"
#include "visitor.hpp"
#include <algorithm>

using namespace lorelai::parser;
void branch::visitchildren(visitor &visit, void *data) {
	std::vector<std::shared_ptr<node>> deleted;

	for (auto &child : children) {
		if (child->accept(visit, child, data)) {
			deleted.push_back(child);
		}
	}

	for (auto &del : deleted) {
		children.erase(std::find(children.begin(), children.end(), del));
	}
}

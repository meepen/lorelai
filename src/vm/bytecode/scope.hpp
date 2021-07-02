#ifndef BYTECODE_SCOPE_HPP_
#define BYTECODE_SCOPE_HPP_

#include "types.hpp"
#include "stack.hpp"
#include <memory>
#include <unordered_map>

namespace lorelai {
	namespace bytecode {
		class scope {
		public:
			scope *findvariablescope(string name, std::shared_ptr<scope> highest = nullptr) {
				auto var = variables.find(name);
				if (var != variables.end()) {
					return this;
				}

				if (parent && parent != highest) {
					return parent->findvariablescope(name);
				}

				return nullptr;
			}

			size_t addvariable(string name, size_t stackpos) {
				variables[name] = stackpos;
				return stackpos;
			}

			size_t getvariableindex(string name) {
				auto var = variables.find(name);
				if (var != variables.end()) {
					return var->second;
				}

				return -1;
			}

			size_t hasvariable(string name) {
				return variables.find(name) != variables.end();
			}

		public:
			std::unordered_map<string, size_t> variables;
			std::shared_ptr<scope> parent;
		};
	}
}

#endif // BYTECODE_SCOPE_HPP_
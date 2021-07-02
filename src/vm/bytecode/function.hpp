#ifndef BYTECODE_FUNCTION_HPP_
#define BYTECODE_FUNCTION_HPP_

#include "bytecode.hpp"
#include "types.hpp"
#include "scope.hpp"
#include <memory>
#include <vector>

namespace lorelai {
	namespace bytecode {
		class function {
		public:
			function() {}
			function(std::shared_ptr<function> &_parent) : parent(_parent), parentscope(_parent->curscope) { }

			std::shared_ptr<scope> &pushscope() {
				auto newscope = std::make_shared<scope>();
				newscope->parent = curscope;
				curscope = newscope;
				return curscope;
			}

			std::shared_ptr<scope> &popscope() {
				if (!curscope) {
					throw;
				}

				for (auto &stackpos : curscope->variables) {
					// erase stack usage for scope now that is gone
					funcstack.freeslots(stackpos.second, 1);
				}

				curscope = curscope->parent;
				return curscope;
			}

			bool haslocal(string name) {
				return curscope->findvariablescope(name, firstscope) != nullptr;
			}

			bool hasupvalue(string name) {
				return parentscope && parentscope->hasvariable(name) || parent && parent->hasupvalue(name);
			}

			// finds a local variable and returns a stack position
			std::uint32_t findlocal(string name) {
				auto found = curscope->findvariablescope(name, firstscope);
				if (!found) {
					// not in current scope, what do?
					// return -1 for now to signify it's not here i guess??
					return -1;
				}

				return found->getvariableindex(name);
			}

			std::uint32_t createlocal(string name) {
				// since it already exists and won't be used passed this point, reuse slot
				// this might have to change if used as an upvalue?
				if (curscope->hasvariable(name)) {
					return curscope->getvariableindex(name);
				}

				auto scopeindex = curscope->addvariable(name, funcstack.getslots(1));
				return scopeindex;
			}

			std::uint32_t createlocals(std::vector<string> names) {
				if (names.size() == 0) {
					return 0;
				}

				auto target = funcstack.getslots(names.size());
				auto varindex = target;

				for (auto &name : names) {
					if (curscope->hasvariable(name)) {
						throw;
					}

					curscope->addvariable(name, varindex++);
				}

				return target;
			}

			std::uint32_t gettemp(size_t amount = 1) {
				return funcstack.getslots(amount);
			}

			void freetemp(std::uint32_t slot, std::uint32_t amount = 1) {
				funcstack.freeslots(slot, amount);
			}

			// returns pair<upvalueprotoid, stackpos>
			std::pair<size_t, size_t> findupvalue(string name) {

			}

			int add(string str) {
				auto found = strings.find(str); 
				if (found != strings.end()) {
					return found->second;
				}

				auto ret = strings[str] = proto->strings_size();
				proto->add_strings(str);
				return ret;
			}

			int add(number num) {
				auto found = numbers.find(num); 
				if (found != numbers.end()) {
					return found->second;
				}

				auto ret = numbers[num] = proto->numbers_size();
				proto->add_numbers(num);
				return ret;
			}

		public:
			stack funcstack;
			std::shared_ptr<function> parent = nullptr;
			std::shared_ptr<scope> curscope = std::make_shared<scope>();
			std::shared_ptr<scope> firstscope = curscope;
			std::shared_ptr<scope> parentscope = nullptr;

			std::shared_ptr<bytecode::prototype> proto = std::make_shared<bytecode::prototype>();
			std::unordered_map<string, int> strings;
			std::unordered_map<number, int> numbers;
		};
	}
}

#endif // BYTECODE_FUNCTION_HPP_
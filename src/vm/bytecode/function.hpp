#ifndef BYTECODE_FUNCTION_HPP_
#define BYTECODE_FUNCTION_HPP_

#include "bytecode.hpp"
#include "types.hpp"
#include "stack.hpp"
#include <memory>
#include <vector>

namespace lorelai {
	namespace bytecode {
		class function : public stack {
		public:
			function() {}

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

			void newstackvariable(const string &name) {
				varlookup[name] = getslots(1);
			}

			void newstackvariables(const std::vector<string> &list) {
				auto index = getslots(list.size());
				for (auto &name : list) {
					varlookup[name] = index++;
				}
			}

			void freestackvariable(const string &name) {
				freeslots(varlookup[name], 1);
				varlookup.erase(name);
			}

			bool hasvariable(const string &name) {
				return varlookup.find(name) != varlookup.end();
			}

		public:
			std::shared_ptr<bytecode::prototype> proto = std::make_shared<bytecode::prototype>();
			std::unordered_map<string, int> strings;
			std::unordered_map<number, int> numbers;
			std::unordered_map<string, std::uint32_t> varlookup;
		};
	}
}

#endif // BYTECODE_FUNCTION_HPP_
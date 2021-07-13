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
			function(function *_parent = nullptr) : parent(_parent) {
				if (parent) {
					proto = parent->proto->add_protos();
				}
				else {
					proto = new bytecode::prototype();
				}
			}
			~function() { }

			bytecode::prototype * release() {
				if (parent) {
					throw;
				}
				auto r = proto;
				r->set_stacksize(maxsize);
				proto = nullptr;
				return r;
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
			bytecode::prototype *proto;
			std::unordered_map<string, int> strings;
			std::unordered_map<number, int> numbers;
			std::unordered_map<string, std::uint32_t> varlookup;
			std::vector<function> children;

			function *parent;
		};
	}
}

#endif // BYTECODE_FUNCTION_HPP_
#ifndef BYTECODE_FUNCTION_HPP_
#define BYTECODE_FUNCTION_HPP_

#include "bytecode.hpp"
#include "types.hpp"
#include "stack.hpp"
#include "scope.hpp"
#include <memory>
#include <vector>

namespace lorelai {
	namespace bytecode {
		class function : public stack {
		public:
			function(function *_parent = nullptr) : parent(_parent) {
				if (parent) {
					protoid = parent->proto.protos.size();
					parent->proto.protos.emplace_back();
				}
			}
			~function() {
				if (parent) {
					parent->proto.protos[protoid] = finalize();
				}
			}

			bytecode::prototype finalize() {
				proto.stacksize = maxsize;
				return proto;
			}

			int add(string str) {
				auto found = strings.find(str); 
				if (found != strings.end()) {
					return found->second;
				}

				auto ret = strings[str] = proto.strings.size();
				proto.strings.emplace_back(str);

				return ret;
			}

			int add(number num) {
				auto found = numbers.find(num); 
				if (found != numbers.end()) {
					return found->second;
				}

				auto ret = numbers[num] = proto.numbers.size();
				proto.numbers.emplace_back(num);

				return ret;
			}

			void newargument(const string &name, std::uint32_t position) {
				varlookup[name] = position;
				maxsize = argcount = std::max(position, argcount) + 1;
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
			bytecode::prototype proto;
			std::uint32_t argcount = 0;
			std::uint32_t protoid;
			std::unordered_map<string, int> strings;
			std::unordered_map<number, int> numbers;
			std::unordered_map<string, std::uint32_t> varlookup;
			std::vector<function> children;

			function *parent;
		};
	}
}

#endif // BYTECODE_FUNCTION_HPP_
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
					proto = parent->proto;
					protoid = parent->proto->protos.size();
					parent->proto->protos.emplace_back();
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
				r->stacksize = maxsize;
				proto = nullptr;
				return r;
			}

			int add(string str) {
				auto found = strings.find(str); 
				if (found != strings.end()) {
					return found->second;
				}

				auto ret = strings[str] = getproto()->strings.size();
				getproto()->strings.emplace_back(str);

				return ret;
			}

			int add(number num) {
				auto found = numbers.find(num); 
				if (found != numbers.end()) {
					return found->second;
				}

				auto ret = numbers[num] = getproto()->numbers.size();
				getproto()->numbers.emplace_back(num);

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

		bytecode::prototype *getproto() {
			if (protoid) {
				return &proto->protos[protoid.value()];
			}
			else {
				return proto;
			}
		}

		public:
			bytecode::prototype *proto;
			optional<std::uint32_t> protoid;
			std::unordered_map<string, int> strings;
			std::unordered_map<number, int> numbers;
			std::unordered_map<string, std::uint32_t> varlookup;
			std::vector<function> children;

			function *parent;
		};
	}
}

#endif // BYTECODE_FUNCTION_HPP_
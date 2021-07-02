#ifndef BYTECODE_STACK_HPP_
#define BYTECODE_STACK_HPP_

#include <list>
#include <stdint.h>

namespace lorelai {
	namespace bytecode {
		class stack {
		public:
			std::uint32_t getslots(std::uint32_t amount) {
				bool found = false;
				for (std::uint32_t index = 0; index <= maxsize; index++) {
					found = true;
					for (std::uint32_t i = 0; i < amount; i++) {
						if (isslotfree(index + i)) {
							found = false;
							break;
						}
					}

					if (found) {
						// obtain and return
						for (std::uint32_t i = 0; i < amount; i++) {
							if (index + i < maxsize) {
								unusedslots.remove(index + i);
							}
						}

						if (index + amount > maxsize) {
							maxsize = index + amount;
						}

						return index;
					}
				}

				throw;
			}

			void freeslots(std::uint32_t slot, std::uint32_t amount) {
				for (std::uint32_t i = 0; i < amount; i++) {
					unusedslots.insert(std::lower_bound(unusedslots.begin(), unusedslots.end(), slot + i), slot + i);
				}
			}

			// note: untested
			std::uint32_t highestfree() {
				auto ret = maxsize - 1;
				if (!isslotfree(ret)) {
					return maxsize;
				}

				while (isslotfree(ret) && ret-- != 0);

				return ret + 1;
			}

			bool isslotfree(std::uint32_t slot) {
				if (slot >= maxsize) {
					return false;
				}

				auto it = std::lower_bound(unusedslots.begin(), unusedslots.end(), slot);

				return it == unusedslots.end() || *it != slot;
			}

		public:
			std::uint32_t maxsize = 0;
			std::list<std::uint32_t> unusedslots;
		};
	}
}

#endif // BYTECODE_STACK_HPP_
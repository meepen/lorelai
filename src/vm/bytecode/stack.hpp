#ifndef BYTECODE_STACK_HPP_
#define BYTECODE_STACK_HPP_

#include <list>
#include <stdint.h>

namespace lorelai {
	namespace bytecode {
		class stack {
		public:
			size_t getslots(size_t amount) {
				bool found = false;
				for (size_t index = 0; index <= maxsize; index++) {
					found = true;
					for (size_t i = 0; i < amount; i++) {
						if (isslotfree(index + i)) {
							found = false;
							break;
						}
					}

					if (found) {
						// obtain and return
						for (size_t i = 0; i < amount; i++) {
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

			void freeslots(size_t slot, size_t amount) {
				for (size_t i = 0; i < amount; i++) {
					unusedslots.insert(std::lower_bound(unusedslots.begin(), unusedslots.end(), slot + i), slot + i);
				}
			}

			// note: untested
			size_t highestfree() {
				size_t ret = maxsize - 1;
				if (!isslotfree(ret)) {
					return maxsize;
				}

				while (isslotfree(ret) && ret-- != 0);

				return ret + 1;
			}

			bool isslotfree(size_t slot) {
				if (slot >= maxsize) {
					return false;
				}

				auto it = std::lower_bound(unusedslots.begin(), unusedslots.end(), slot);

				return it == unusedslots.end() || *it != slot;
			}

		public:
			size_t maxsize = 0;
			std::list<size_t> unusedslots;
		};
	}
}

#endif // BYTECODE_STACK_HPP_
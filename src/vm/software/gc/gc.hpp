#ifndef GC_HPP_
#define GC_HPP_

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdlib.h>

namespace lorelai {
	namespace gc {
		class manager {
			using byte = unsigned char;
			struct heap {
				std::size_t size;
				byte *freeptr;
				bool mode = true; // true = top-down, false = bottom-up
				byte data[0];

				byte *allocate(std::size_t s) {
					if (mode) {
						auto ret = freeptr;
						freeptr += s;
						return ret;
					}
					else {
						freeptr -= s;
						return freeptr;
					}
				}

				bool hasfree(std::size_t s) {
					if (mode) {
						return freeptr + s < data + size;
					}
					else {
						return freeptr - s >= data;
					}
				}
			};

		private:
			manager(const manager &other) { }
			manager(manager &&other) { }

		public:
			manager() {
				newheap(1024u * 1024u * 64);
			}

			struct infoheader {
				std::size_t size;
				std::size_t hash;
				std::uint8_t type;
				std::uint8_t mark;
				byte data[0];

				template <typename T>
				T *get() {
					return reinterpret_cast<T *>(data);
				}
			};

			infoheader *allocate(std::size_t size) {
				auto head = reinterpret_cast<infoheader *>(reserve(size + sizeof(infoheader)));
				head->size = size + sizeof(infoheader);
				head->hash = nexthash();

				return head;
			}

			template <typename T, typename... _Args>
			infoheader *allocate(std::size_t type, _Args &&...args) {
				auto info = allocate(sizeof(T));
				info->type = type;

				new (reinterpret_cast<T *>(info->data)) T(std::forward<_Args>(args)...);

				return info;
			}

			std::size_t hash(void *dat) {
				infoheader *info = reinterpret_cast<infoheader *>(reinterpret_cast<byte *>(dat) - sizeof(infoheader));
				return info->hash;
			}

			std::size_t size(void *dat) {
				infoheader *info = reinterpret_cast<infoheader *>(reinterpret_cast<byte *>(dat) - sizeof(infoheader));
				return info->size;
			}

			std::size_t type(void *dat) {
				infoheader *info = reinterpret_cast<infoheader *>(reinterpret_cast<byte *>(dat) - sizeof(infoheader));
				return info->type;
			}

		private:
			byte *reserve(std::size_t s) {
				for (std::size_t i = 0; i < heapcount; i++) {
					auto heap = heaps[i];
					if (heap->hasfree(s)) {
						return heap->allocate(s);
					}
				}

				// TODO: gc or create new heap
				throw;
			}

			heap *getheap(infoheader *info) {
				for (std::size_t i = 0; i < heapcount; i++) {
					auto *heap = heaps[i];
					if (reinterpret_cast<byte *>(info) >= heap->data && reinterpret_cast<byte *>(info) < heap->data + heap->size) {
						return heap;
					}
				}

				return nullptr;
			}

			std::size_t nexthash() {
				return curhash++;
			}

			heap *newheap(std::size_t s) {
				heapcount++;
				heaps = reinterpret_cast<heap **>(realloc(heaps, sizeof(heap *) * heapcount));
				auto data = reinterpret_cast<byte *>(malloc(s + sizeof(heap)));
				auto newheap = reinterpret_cast<heap *>(data);
				newheap->size = s;
				newheap->freeptr = data + sizeof(heap);
				newheap->mode = true;
				heaps[heapcount - 1] = newheap;
			}

		private:
			heap **heaps = nullptr;
			std::size_t heapcount = 0;

			std::size_t curhash = 0;

			std::map<std::size_t, infoheader *> hashmap;
		};
	}
}

#endif // GC_HPP_
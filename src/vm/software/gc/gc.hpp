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
				newheap(1024u * 1024u * 64u);
			}
			~manager() {
				for (std::size_t i = 0; i < heapcount; i++) {
					free(heaps[i]);
				}
				free(heaps);
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
			infoheader *allocate(std::uint8_t type, _Args &&...args) {
				auto info = allocate(sizeof(T));
				info->type = type;

				new (info->get<T *>()) T(std::forward<_Args>(args)...);

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
				newheap->freeptr = newheap->data;
				newheap->mode = true;
				heaps[heapcount - 1] = newheap;

				return newheap;
			}

		private:
			struct Iterator 
			{
				using iterator_category = std::forward_iterator_tag;
				using difference_type   = std::ptrdiff_t;
				using value_type        = infoheader *;
				using pointer           = value_type;
				using reference         = value_type;

    			Iterator(manager *__manager, bool end) : _manager(__manager) {
					if (end) {
						curheap = _manager->heapcount;
						_info = nullptr;
					}
					else {
						curheap = 0;
						if (getheap()->mode) {
							_info = reinterpret_cast<infoheader *>(getheap()->data);
						}
						else {
							_info = reinterpret_cast<infoheader *>(getheap()->freeptr);
						}
						if (getheap()->mode ? reinterpret_cast<byte *>(_info) == getheap()->freeptr : reinterpret_cast<byte *>(_info) == getheap()->data + getheap()->size) {
							curheap++;
							_info = nullptr;
						}
					}
				}

				reference operator*() const { return _info; }
				pointer operator->() { return _info; }

				Iterator& operator++() {
					auto _heap = getheap();
					if (!_heap->mode && reinterpret_cast<byte *>(_info) == _heap->freeptr) {
						curheap++;
						if (_manager->heapcount == curheap) {
							_info = nullptr;
						}
						else {
							_info = reinterpret_cast<infoheader *>(getheap()->freeptr);
						}
						return *this;
					}
					_info = reinterpret_cast<infoheader *>(reinterpret_cast<byte *>(_info) + _info->size);
					if (_heap->mode && reinterpret_cast<byte *>(_info) == _heap->freeptr) {
						curheap++;
						if (_manager->heapcount == curheap) {
							_info = nullptr;
						}
						else {
							_info = reinterpret_cast<infoheader *>(getheap()->data);
						}
					}
					return *this;
				}
				Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

				friend bool operator== (const Iterator& a, const Iterator& b) { return a._info == b._info; };
				friend bool operator!= (const Iterator& a, const Iterator& b) { return a._info != b._info; };     

			private:

				heap *getheap() {
					return _manager->heaps[curheap];
				}

				manager *_manager;
				std::size_t curheap;
				pointer _info;
			};

		public:
			Iterator begin() { return Iterator(this, false); }
			Iterator end() { return Iterator(this, true); }

		private:
			heap **heaps = nullptr;
			std::size_t heapcount = 0;

			std::size_t curhash = 0;

			std::map<std::size_t, infoheader *> hashmap;
		};
	}
}

#endif // GC_HPP_
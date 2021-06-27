#ifndef CONTAINER_HPP_
#define CONTAINER_HPP_

#include <cassert>
#include <type_traits>
#include <memory>
#include <vector>
#include <string>

namespace lorelai {
	namespace vm {
		namespace software {
			template <class T>
			struct _reference;

			template <class T>
			class deleter {
			public:
				virtual void operator()(_reference<T> *) = 0;
			};

			template <class T>
			struct _reference {
				int count = 0;
				T *data = nullptr;
				deleter<T> *del = nullptr;
			};

			template <class T>
			class container {
			public:
				container &operator=(container &&other) noexcept {
        			std::swap(ref, other.ref);
        			std::swap(ptr, other.ptr);
					return *this;
				}
				container() { }

				template <class V>
				container(const _reference<V> *other) : ref(other), ptr(other ? other->data : other) {
					static_assert(std::is_convertible<V *, T *>::value, "must be baseclass");
					ref->count++;
				}
				template <class V, typename... _Args>
				container(_reference<V> *other, _Args&&... __args) : ref(other), ptr(other->data) {
					static_assert(std::is_convertible<V *, T *>::value, "must be baseclass");
					ref->count++;
					*ref->data = T(std::forward<_Args>(__args)...);
				}

				template <class V>
				container(const container<V> &other) : ref(reinterpret_cast<_reference<T> *>(other.ref)), ptr(reinterpret_cast<T *>(other.ptr)) {
					static_assert(std::is_convertible<V *, T *>::value, "must be baseclass");
					ref->count++;
				}

				container(const container<T> &other) : ref(other.ref), ptr(other.ptr) {
					if (ref) {
						ref->count++;
					}
				}
				
				container &operator=(const container &other)
				{
					return *this = container(other);
				}
				
    			container(container&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)), ref(std::exchange(other.ref, nullptr)) { }

				container(std::nullptr_t) : ref(nullptr), ptr(nullptr) { }

				bool operator==(const container<T> &other) const {
					return other.ref == ref;
				}

				operator bool() const {
					return ptr != nullptr;
				}

				~container() {
					if (!ref) {
						return;
					}
					auto count = --ref->count;
					assert(count >= 0);
					if (count == 0) {
						(*ref->del)(ref);
					}
				}

				T *get() const {
					return ptr;
				}

				void *unique() {
					return reinterpret_cast<void *>(&ref->count);
				}

				T *operator->() const {
					return ptr;
				}

			public:
				_reference<T> *ref = nullptr;
				T *ptr = nullptr;
			};

			template <typename T, size_t peralloc = 256u>
			class allocator {
				struct block {
					struct item {
						_reference<T> ref;
						block *base = nullptr;
					};
					size_t references = 0;
					item refs[peralloc];
					T data[peralloc];
				};

				class _deleter : public deleter<T> {
				public:
					_deleter(allocator<T, peralloc> *_allocator) : alloc(_allocator) {
					}

					void operator()(_reference<T> *ref) override {
						alloc->free(ref);
					}

				public:
					allocator<T, peralloc> *alloc;
				};

				class _cleanup : public deleter<T> {
				public:
					void operator()(_reference<T> *ref) override {
						auto item = reinterpret_cast<typename block::item *>(ref);

						auto base = item->base;
						if (--base->references == 0) {
							delete base;
							if (--references == 0) {
								delete this;
							}
						}
					}

				public:
					int references = 0;
				};

			public:
				allocator() : del(this) {
				}
				~allocator() {
					auto cleanup = new _cleanup;
					for (auto &blk : allocated) {
						if (blk->references > 0) {
							cleanup->references++;
							for (size_t i = 0; i < peralloc; i++) {
								blk->refs[i].ref.del = cleanup;
							}
						}
						else {
							delete blk;
						}
					}
					if (cleanup->references == 0) {
						delete cleanup;
					}
				}
				
				template <typename... _Args>
				container<T> take(_Args&&... __args) {
					_reference<T> *data;
					if (unused.size() > 0) {
						data = unused.back();
						unused.pop_back();
					}
					else {
						if (used == peralloc) {
							allocated.push_back(new block);
							auto blk = allocated.back();
							for (size_t i = 0; i < peralloc; i++) {
								auto &ref = blk->refs[i];
								ref.ref.data = &blk->data[i];
								ref.ref.del = &del;
								ref.base = blk;
							}
							used = 0;
						}

						auto blk = allocated.back();
						data = &blk->refs[used++].ref;
					}

					auto blk = reinterpret_cast<typename block::item *>(data)->base;
					data->count = 0;
					blk->references++;

					return container<T>(data, std::forward<_Args>(__args)...);
				}

				void free(_reference<T> *data) {
					unused.push_back(data);
					auto blk = reinterpret_cast<typename block::item *>(data)->base;
					blk->references--;
				}

			public:
				_deleter del;
				size_t used = peralloc;
				std::vector<_reference<T> *> unused;
				std::vector<block *> allocated;
			};
		}
	}
}

namespace std {
	template <typename T>
	struct hash<lorelai::vm::software::container<T>> {
		size_t operator()(const lorelai::vm::software::container<T> &x) const {
			return std::hash<T *>()(x.get());
		}
	};
}

#endif // CONTAINER_HPP_
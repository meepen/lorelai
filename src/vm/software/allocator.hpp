#include <type_traits>
#include <memory>

template <typename T>
struct _reference {
	int count = 0;
	T *data = nullptr;
	std::default_delete<T> _delete
};


template <class T, class _Deleter = std::default_delete<T>, class _Allocator = std::allocator<T>>
class container {
public:
	template <class C>
	container(const container<C> &other) {
		ref.count++;
		static_assert(std::is_convertible<C *, T *>, "copy constructor must be convertible");
	}

	~container() {
		if (--ref.count == 0) {

		}
	}

	T *&operator->() {
		return ref.reference;
	}

public:
	_reference<T> ref;
};

template <typename T>
class objectallocator {
public:
	objectallocator(int _peralloc = 64) : peralloc(_peralloc), cache(peralloc) {
		cache.resize(peralloc);
	}

	container<T> take() {
		if (cache.size() == 0) {
			cache.resize(peralloc);
		}
	}

public:
	std::vector<T> cache;
};
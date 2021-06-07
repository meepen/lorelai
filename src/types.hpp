#include <string>
#include <experimental/optional>

namespace lorelai {
	template <class T>
	using optional = std::experimental::optional<T>;

	using number = double;
	using string = std::string;
}
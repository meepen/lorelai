#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <string>
#include <experimental/optional>
#include <exception>

#ifndef LORELAI_INLINE
#define LORELAI_INLINE
#warning force inline disabled
#endif

namespace lorelai {
	template <class T>
	using optional = std::experimental::optional<T>;

	using number = double;
	using string = std::string;

	class exception : public std::exception {
	public:
		exception(string str) : data(str) { }

		const char *what() const noexcept override {
			return data.c_str();
		}

	public:
		string data;
	};

	static number tonumber(string str, optional<int> base = {}) {
		number num;
		size_t size = 0;
		if (str.size() >= 2 && str[0] == '0' && (!base || base.value() == 16)) {
			char typ = str[1];
			if (typ == 'x') {
				char *endptr;
				num = std::strtod(str.c_str(), &endptr);
				if ((endptr - str.c_str()) != str.size()) {
					throw exception("invalid character for hexnumber: " + string(endptr[0], 1));
				}
				size = str.size();
			}
			else if (typ == 'b' && (!base || base.value() == 2)) {
				num = static_cast<number>(std::stol(str.substr(2), &size, 2));
				size += 2;
			}
		}

		if (size == 0 && (!base || base.value() == 10)) {
			num = std::stod(str, &size);
		}

		if (size != str.size()) {
			throw exception("invalid character for number: " + string(str[size], 1));
		}

		return num;
	}
}

#endif // TYPES_HPP_


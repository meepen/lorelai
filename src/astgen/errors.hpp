#ifndef ERRORS_HPP_
#define ERRORS_HPP_

#include <string>
#include <exception>

namespace lorelai {
	namespace astgen {
		namespace error {
			class unexpected_for : public std::exception {
			public:
				unexpected_for(std::string data, std::string parsing) : error("Unexpected data while parsing " + parsing + ": " + data) { }
			public:
				const char *what() const noexcept override {
					return error.c_str();
				}
			private:
				std::string error;
			};

			class expected_for : public std::exception {
			public:
				expected_for(std::string expected, std::string parsing, std::string got) : error("Expected " + expected + " while parsing " + parsing + " but received " + got) { }
			public:
				const char *what() const noexcept override {
					return error.c_str();
				}
			private:
				std::string error;
			};
		}
	}
}

#endif // ERRORS_HPP_
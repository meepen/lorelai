#ifndef LORELAI_BYTEBUFFER_HPP_
#define LORELAI_BYTEBUFFER_HPP_

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

namespace lorelai {
	namespace bytecode {
		class writebuffer;
		template <typename T>
		struct write {
			void operator()(const T &, writebuffer &buffer) const {
				static_assert("not implemented");
			}
		};
		class readbuffer;
		template <typename T>
		struct read {
			void operator()(T &out, readbuffer &buffer) const {
				static_assert("not implemented");
			}
		};

		class writebuffer {
		public:
			struct _posdata {
				std::uint64_t position;
			};

			template <typename T>
			void write(const T &data) {
				bytecode::write<T>()(data, *this);
			}

			_posdata writei8(const std::uint8_t &n) {
				_posdata ret = { data.size() };
				data.push_back(n);

				return ret;
			}

			_posdata writei16(const std::uint16_t &n) {
				auto r = writei8(n >> 8);
				writei8(n);

				return r;
			}

			_posdata writei32(const std::uint32_t &n) {
				auto r = writei16(n >> 16);
				writei16(n);

				return r;
			}

			_posdata writei64(const std::uint64_t &n) {
				auto r = writei32(n >> 32);
				writei32(n);

				return r;
			}

		public:
			std::vector<std::uint8_t> data;
		};

		class readbuffer {
		public:
			~readbuffer() {
				delete[] data;
				data = ptr = nullptr;
				size = 0;
			}
			readbuffer(const std::uint8_t *_data, std::uint64_t _size) : size(_size) {
				data = new std::uint8_t[size];
				std::memcpy(data, _data, size);
			}
			readbuffer(std::vector<std::uint8_t> data) : readbuffer(data.data(), data.size()) { }
			readbuffer(std::string data) : readbuffer(reinterpret_cast<const std::uint8_t *>(data.c_str()), data.size()) { }
			template<typename T>
			void read(T &out) {
				bytecode::read<T>()(out, *this);
			}

			template <typename T>
			T read() {
				T out;
				bytecode::read<T>()(out, *this);

				return out;
			}

			std::uint8_t readi8() {
				return *ptr++;
			}

			std::uint16_t readi16() {
				auto n0 = static_cast<std::uint16_t>(readi8());
				return (n0 << 8) | readi8();
			}

			std::uint32_t readi32() {
				auto n0 = static_cast<std::uint32_t>(readi16());
				return (n0 << 16) | readi16();
			}

			std::uint64_t readi64() {
				auto n0 = static_cast<std::uint64_t>(readi32());
				return (n0 << 32) | readi32();
			}

		public:
			std::uint64_t size;
			std::uint8_t *data;
			std::uint8_t *ptr;
		};

		template <>
		struct write<std::uint8_t> {
			void operator()(const std::uint8_t &data, writebuffer &buffer) const {
				buffer.writei8(data);
			}
		};
		template <>
		struct write<std::int8_t> {
			void operator()(const std::int8_t &data, writebuffer &buffer) const {
				buffer.writei8(static_cast<uint8_t>(data));
			}
		};
		template <>
		struct write<std::uint16_t> {
			void operator()(const std::uint16_t &data, writebuffer &buffer) const {
				buffer.writei16(data);
			}
		};
		template <>
		struct write<std::int16_t> {
			void operator()(const std::int16_t &data, writebuffer &buffer) const {
				buffer.writei16(static_cast<uint16_t>(data));
			}
		};
		template <>
		struct write<std::uint32_t> {
			void operator()(const std::uint32_t &data, writebuffer &buffer) const {
				buffer.writei32(data);
			}
		};
		template <>
		struct write<std::int32_t> {
			void operator()(const std::int32_t &data, writebuffer &buffer) const {
				buffer.writei32(static_cast<uint32_t>(data));
			}
		};
		template <>
		struct write<std::uint64_t> {
			void operator()(const std::uint64_t &data, writebuffer &buffer) const {
				buffer.writei64(data);
			}
		};
		template <>
		struct write<std::int64_t> {
			void operator()(const std::int64_t &data, writebuffer &buffer) const {
				buffer.writei64(static_cast<uint64_t>(data));
			}
		};

		template <>
		struct write<char> {
			void operator()(const char &data, writebuffer &buffer) const {
				buffer.writei8(data);
			}
		};
		template <>
		struct write<float> {
			void operator()(const float &data, writebuffer &buffer) const {
				std::uint32_t i;
				std::memcpy(&i, &data, sizeof(data));
				buffer.writei32(i);
			}
		};
		template <>
		struct write<double> {
			void operator()(const double &data, writebuffer &buffer) const {
				std::uint64_t i;
				std::memcpy(&i, &data, sizeof(data));
				buffer.writei64(i);
			}
		};

		// READERS

		template <>
		struct read<std::uint8_t> {
			void operator()(std::uint8_t &out, readbuffer &buffer) const {
				out = buffer.readi8();
			}
		};
		template <>
		struct read<std::int8_t> {
			void operator()(std::int8_t &out, readbuffer &buffer) const {
				out = static_cast<std::int8_t>(buffer.readi8());
			}
		};
		template <>
		struct read<std::uint16_t> {
			void operator()(std::uint16_t &out, readbuffer &buffer) const {
				out = buffer.readi16();
			}
		};
		template <>
		struct read<std::int16_t> {
			void operator()(std::int16_t &out, readbuffer &buffer) const {
				out = static_cast<std::int16_t>(buffer.readi16());
			}
		};
		template <>
		struct read<std::uint32_t> {
			void operator()(std::uint32_t &out, readbuffer &buffer) const {
				out = buffer.readi32();
			}
		};
		template <>
		struct read<std::int32_t> {
			std::int32_t operator()(readbuffer &buffer) const {
				return static_cast<std::int32_t>(buffer.readi32());
			}
		};
		template <>
		struct read<std::uint64_t> {
			void operator()(std::uint64_t &out, readbuffer &buffer) const {
				out = buffer.readi64();
			}
		};
		template <>
		struct read<std::int64_t> {
			void operator()(std::int64_t &out, readbuffer &buffer) const {
				out = static_cast<std::int64_t>(buffer.readi64());
			}
		};
		

		template <>
		struct read<float> {
			void operator()(float &out, readbuffer &buffer) const {
				auto n = buffer.readi32();
				std::memcpy(&out, &n, sizeof(n));
			}
		};
		template <>
		struct read<char> {
			void operator()(char &out, readbuffer &buffer) const {
				out = buffer.readi8();
			}
		};
		template <>
		struct read<double> {
			void operator()(double &out, readbuffer &buffer) const {
				auto n = buffer.readi64();
				std::memcpy(&out, &n, sizeof(n));
			}
		};
		
		// std::string
		template <>
		struct write<std::string> {
			void operator()(const std::string &str, writebuffer &buffer) const {
				buffer.write<std::uint64_t>((std::uint64_t)str.size());
				for (auto c : str) {
					buffer.write(c);
				}
			}
		};

		template <>
		struct read<std::string> {
			void operator()(std::string &out, readbuffer &buffer) const {
				auto size = buffer.readi64();
				std::vector<char> read;
				read.resize(size);

				for (std::uint64_t i = 0; i < size; i++) {
					read[i] = buffer.read<char>();
				}

				out = std::string(read.begin(), read.end());
			}
		};
		
		// std::vector<T>
		template <typename T>
		struct write<std::vector<T>> {
			void operator()(const std::vector<T> &data, writebuffer &buffer) const {
				buffer.write<std::uint64_t>(data.size());
				for (auto &c : data) {
					buffer.write<T>(c);
				}
			}
		};

		template <typename T>
		struct read<std::vector<T>> {
			void operator()(std::vector<T> &read, readbuffer &buffer) const {
				auto size = buffer.read<std::uint64_t>();
				read.resize(0);
				read.reserve(size);

				for (std::uint64_t i = 0; i < size; i++) {
					read.push_back(buffer.read<T>());
				}
			}
		};
	}
}

#endif // LORELAI_BYTEBUFFER_HPP_
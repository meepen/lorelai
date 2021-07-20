#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include <cstdint>
#include <memory>
#include <sstream>
#include <cmath>
#include <sstream>

#include "state.hpp"
#include "types.hpp"
#include "bytecode.hpp"
#include <memory>
#include <unordered_map>

#define LORELAI_SOFTWARE_DEFAULT_FUNCTION(name, ...) name (__VA_ARGS__) { throw exception(string("cannot " #name " a ") + typenames[_typeid()]); }


namespace lorelai {
	namespace vm {
		static_assert(-1 == ~0, "must be two's complement");

		enum boxed_type {
			BOXED_TYPE_NUMBER = 0,
			BOXED_TYPE_NIL,
			BOXED_TYPE_REFERENCE, // MUST BE POWER OF TWO
			BOXED_TYPE_TRUE,
			BOXED_TYPE_FALSE,
		};
		static_assert(sizeof(number) == sizeof(std::uint64_t), "number must be 64 bit");

		LORELAI_INLINE constexpr std::uint64_t encodetype(const boxed_type t) {
			return (static_cast<std::uint64_t>(t) << 48) | (static_cast<std::uint64_t>(0x7FF) << 52);
		}
		LORELAI_INLINE constexpr boxed_type decodetype(std::uint64_t d) {
			return static_cast<boxed_type>((d >> 48) & 7);
		}
		LORELAI_INLINE constexpr bool hasencodedtype(std::uint64_t d) {
			return ((d >> 52) & 0x7FF) == 0x7FF;
		}
		LORELAI_INLINE constexpr std::uint64_t exponents(std::uint64_t a) {
			return a & (static_cast<std::uint64_t>(0x7FF) << 52);
		}

		class softwarestate;
		using luafunction = int (*)(softwarestate &state, int nargs);
		struct object;

		class referenceobject {
		public:
			virtual ~referenceobject() { }
			virtual data_type _typeid() const = 0;

			virtual size_t hash() const {
				return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(this));
			}

			virtual string tostring(softwarestate &state) const {
				return typenames[_typeid()];
			}

			virtual number tonumber(softwarestate &state) {
				throw exception(string("cannot convert ") + typenames[_typeid()] + " to number");
			}

			[[noreturn]] virtual bool            LORELAI_SOFTWARE_DEFAULT_FUNCTION(rawget,      softwarestate &state, object &out, const object &index)
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(rawset,      softwarestate &state, const object &index, const object &data)
			[[noreturn]] virtual state::_retdata LORELAI_SOFTWARE_DEFAULT_FUNCTION(call,        softwarestate &state, int nargs)
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(setindex,    softwarestate &state, object &key, object &value)
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(length,      softwarestate &state, object &out)
			[[noreturn]] virtual bool            LORELAI_SOFTWARE_DEFAULT_FUNCTION(lessthan,    softwarestate &state, object &rhs);
			[[noreturn]] virtual bool            LORELAI_SOFTWARE_DEFAULT_FUNCTION(greaterthan, softwarestate &state, object &rhs);

			// math functions
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(add_rhs,         softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(subtract_rhs,    softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(divide_rhs,      softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(multiply_rhs,    softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(power_rhs,       softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(modulo_rhs,      softwarestate &state, object &out, object &rhs);

			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(add_lhs,         softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(subtract_lhs,    softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(divide_lhs,      softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(multiply_lhs,    softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(power_lhs,       softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(modulo_lhs,      softwarestate &state, object &out, object &rhs);

			// compare functions
			bool equals_rhs(softwarestate &state, object &rhs);
			bool equals_lhs(softwarestate &state, object &rhs);

			inline void concat(softwarestate &state, object &out, object &other);

			virtual bool index(softwarestate &state, object &out, object &index) {
				if (!rawget(state, out, index)) {
					// TODO: metatable access

					return false;
				}

				return true;
			}

			virtual object metatable(softwarestate &state) const = 0;
		};

		struct object {
		public:
			LORELAI_INLINE referenceobject &ref() {
				return *reinterpret_cast<referenceobject *>(data & ~(static_cast<std::uint64_t>(0xFFFF) << 48));
			}

			LORELAI_INLINE boxed_type boxtype() {
				if (!hasencodedtype(data)) {
					return BOXED_TYPE_NUMBER;
				}

				return decodetype(data);
			}

			LORELAI_INLINE bool fast_numbercheck(object &other) {
				return !(hasencodedtype(data) | hasencodedtype(other.data));
			}

		public:
			LORELAI_INLINE object(const object &obj) {
				set(obj);
			}
			LORELAI_INLINE object(referenceobject &ref) {
				set(ref);
			}
			LORELAI_INLINE object(const number num) {
				set(num);
			}
			LORELAI_INLINE object(const bool b) {
				set(b);
			}
			LORELAI_INLINE object() {
				set();
			}

		public:
			LORELAI_INLINE void set(referenceobject &ref) {
				data = encodetype(BOXED_TYPE_REFERENCE) | reinterpret_cast<std::uintptr_t>(&ref);
			}

			LORELAI_INLINE void set(const bool &b) {
				data = (b ? encodetype(BOXED_TYPE_TRUE) : encodetype(BOXED_TYPE_FALSE));
			}

			LORELAI_INLINE void set(const number &n) {
				num = n;
			}

			LORELAI_INLINE void set(const object &other) {
				data = other.data;
			}

			LORELAI_INLINE void set() {
				data = encodetype(BOXED_TYPE_NIL);
			}

			LORELAI_INLINE void unset() {
				set();
			}

			// this is used for c++ structures and cannot be modified
			bool operator==(const object &other) const {
				if (!hasencodedtype(data) & !hasencodedtype(other.data)) {
					return num == other.num;
				}

				return data == other.data;
			}

		public:
			// called in lua vm
			LORELAI_INLINE bool equals        (softwarestate &state, object &other) {
				if (!hasencodedtype(data) & !hasencodedtype(other.data)) {
					return num == other.num;
				}

				if (decodetype(data) & BOXED_TYPE_REFERENCE) {
					return ref().equals_lhs(state, other);
				}
				else if (decodetype(other.data) == BOXED_TYPE_REFERENCE) {
					return other.ref().equals_rhs(state, *this);
				}

				return data == other.data;
			}

			LORELAI_INLINE bool lessthan      (softwarestate &state, object &other) {
				return tonumber(state) < other.tonumber(state);
			}
			LORELAI_INLINE bool greaterthan   (softwarestate &state, object &other) {
				return tonumber(state) > other.tonumber(state);
			}

			LORELAI_INLINE void add(softwarestate &state, object &out, object &other) {
				if (fast_numbercheck(other)) {
					out.set(num + other.num);
				}
				else {
					convertexception("number");
				}
			}

			LORELAI_INLINE void subtract(softwarestate &state, object &out, object &other) {
				if (fast_numbercheck(other)) {
					out.set(num - other.num);
				}
				else {
					convertexception("number");
				}
			}

			LORELAI_INLINE void divide(softwarestate &state, object &out, object &other) {
				if (fast_numbercheck(other)) {
					out.set(num / other.num);
				}
				else {
					convertexception("number");
				}
			}

			LORELAI_INLINE void multiply(softwarestate &state, object &out, object &other) {
				if (fast_numbercheck(other)) {
					out.set(num * other.num);
				}
				else {
					convertexception("number");
				}
			}

			LORELAI_INLINE void power(softwarestate &state, object &out, object &other) {
				if (fast_numbercheck(other)) {
					out.set(std::pow(num, other.num));
				}
				else {
					convertexception("number");
				}
			}

			LORELAI_INLINE void modulo(softwarestate &state, object &out, object &other) {
				if (fast_numbercheck(other)) {
					auto a = num, b = other.num;

					out.set(a - b * std::floor(a / b));
				}
				else {
					convertexception("number");
				}
			}

			LORELAI_INLINE bool rawget(softwarestate &state, object &out, const object &index) {
				if (!hasencodedtype(data) || decodetype(data) != BOXED_TYPE_REFERENCE) {
					throw exception(string("NYI: cannot rawget ") + gettypename());
				}

				return ref().rawget(state, out, index);
			}

			LORELAI_INLINE void rawset(softwarestate &state, const object &index, const object &value) {
				if (!hasencodedtype(data) || decodetype(data) != BOXED_TYPE_REFERENCE) {
					throw exception(string("NYI: cannot rawset ") + gettypename());
				}

				return ref().rawset(state, index, value);
			}
			
			LORELAI_INLINE bool index            (softwarestate &state, object &out, object &index) {
				if (!hasencodedtype(data) || decodetype(data) != BOXED_TYPE_REFERENCE) {
					throw exception(string("NYI: cannot index ") + gettypename());
				}

				return ref().index(state, out, index);
			}
			
			LORELAI_INLINE void setindex            (softwarestate &state, object &key, object &value) {
				if (!hasencodedtype(data) || decodetype(data) != BOXED_TYPE_REFERENCE) {
					throw exception(string("NYI: cannot setindex ") + gettypename());
				}

				return ref().setindex(state, key, value);
			}

			LORELAI_INLINE state::_retdata call  (softwarestate &state, int nargs) {
				if (!hasencodedtype(data) || decodetype(data) != BOXED_TYPE_REFERENCE) {
					throw exception(string("NYI: cannot call ") + gettypename());
				}

				return ref().call(state, nargs);
			}

			[[noreturn]] LORELAI_INLINE void convertexception(const char *what) {
				throw exception(string("cannot convert ") + gettypename() + " to " + what);
			}

			LORELAI_INLINE number tonumber(softwarestate &state) {
				if (!hasencodedtype(data)) {
					return num;
				}
				else if (decodetype(data) == BOXED_TYPE_REFERENCE) {
					return ref().tonumber(state);
				}

				convertexception("number");
			}

			LORELAI_INLINE string tostring(softwarestate &state) {
				if (!hasencodedtype(data)) {
					std::ostringstream stream;
					stream.precision(13);
					stream << num;
					return stream.str();
				}
				switch (decodetype(data)) {
				case BOXED_TYPE_NIL:
					return "nil";
				case BOXED_TYPE_TRUE:
					return "true";
				case BOXED_TYPE_FALSE:
					return "false";
				case BOXED_TYPE_NUMBER:
					return "nan";
				case BOXED_TYPE_REFERENCE:
					return ref().tostring(state);
				}

				throw exception("unknown type");
			}

			LORELAI_INLINE bool tobool(softwarestate &state) {
				if (!hasencodedtype(data)) {
					return true;
				}

				switch (decodetype(data)) {
					case BOXED_TYPE_NIL:
						return false;
					case BOXED_TYPE_FALSE:
						return false;
					default:
						return true;
				}
			}

			LORELAI_INLINE object metatable(softwarestate &state) {
				if (!hasencodedtype(data)) {
					return object(); /* todo */
				}
				switch (decodetype(data)) {
				case BOXED_TYPE_REFERENCE:
					return ref().metatable(state);
				default :
					return object(); /* todo */
				}
			}

		public:
			LORELAI_INLINE const char *gettypename() {
				if (!hasencodedtype(data)) {
					return typenames[NUMBER];
				}
				switch (decodetype(data)) {
				case BOXED_TYPE_REFERENCE:
					return typenames[ref()._typeid()];
				case BOXED_TYPE_FALSE:
					return typenames[BOOL];
				case BOXED_TYPE_TRUE:
					return typenames[BOOL];
				case BOXED_TYPE_NIL:
					return typenames[NIL];
				case BOXED_TYPE_NUMBER:
					return typenames[NUMBER];
				default:
					throw exception("unknown type");
				}
			}

		public:
			union {
				std::uint64_t data = encodetype(BOXED_TYPE_NIL);
				number num;
			};
		};
	}
}

namespace std {
	template<>
	struct hash<lorelai::vm::object> {
		size_t operator()(const lorelai::vm::object &obj) const {
			if (!lorelai::vm::hasencodedtype(obj.data)) {
				return std::hash<lorelai::number>()(obj.num);
			}
			switch (lorelai::vm::decodetype(obj.data)) {
			case lorelai::vm::BOXED_TYPE_NUMBER:
				return std::hash<lorelai::number>()(obj.num);
			default:
				return std::hash<std::uint64_t>()(obj.data);
			}
		}
	};
}

namespace lorelai {
	namespace vm {
		class stringobject : public referenceobject {
		private:
			stringobject() { }
		public:
			stringobject(string _str) : str(_str) { }

			static object create(softwarestate &state, string str);


			data_type _typeid() const override {
				return STRING;
			}

			number tonumber(softwarestate &state) override {
				return lorelai::tonumber(str);
			}

			string tostring(softwarestate &state) const override {
				return str;
			}

			object metatable(softwarestate &state) const override;

		public:
			string str;
		};

		class functionobject : public referenceobject {
		protected:
			functionobject() { }

		public:
			object metatable(softwarestate &state) const override;
		};

		class luafunctionobject : public functionobject {
			struct tabledata {
				std::vector<std::pair<bytecode::tablevalue, bytecode::tablevalue>> hashpart;
				std::vector<bytecode::tablevalue> arraypart;
			};
			void fromtablevalue(object &out, const bytecode::tablevalue &data);
		public:
			static object create(softwarestate &state, const bytecode::prototype &proto);

		public:
			luafunctionobject(softwarestate &state, const bytecode::prototype &proto);
			luafunctionobject();

		public:
			data_type _typeid() const override {
				return LUAFUNCTION;
			}

			state::_retdata call(softwarestate &state, int nargs) override;

		public:
			struct instruction;
			std::shared_ptr<instruction> allocated;
			size_t size;
			std::uint32_t stacksize;
			std::vector<object> strings;
			std::vector<object> numbers;
			std::vector<tabledata> tables;
			std::vector<luafunctionobject> protos;
		};

		class cfunctionobject : public functionobject {
		public:
			static object create(softwarestate &state, luafunction func);

		public:
			cfunctionobject(luafunction func) : data(func) { }
			cfunctionobject() { }

		public:
			data_type _typeid() const override {
				return CFUNCTION;
			}

			state::_retdata call(softwarestate &state, int nargs) override;

		public:
			luafunction data;
		};

		class tableobject : public referenceobject {
		public:
			static object create(softwarestate &state);

		public:
			tableobject() { }

		public:
			data_type _typeid() const override {
				return TABLE;
			}

			object metatable(softwarestate &state) const override {
				return _metatable;
			}

			void rawset(softwarestate &state, const object &lhs, const object &rhs) override {
				data[lhs] = rhs;
			}

			bool rawget(softwarestate &state, object &out, const object &index) override {
				auto found = data.find(index);
				if (found == data.end()) {
					out.unset();
					return false;
				}
				else {
					out.set(found->second);
					return true;
				}
			}

			
			void setindex(softwarestate &state, object &key, object &value) override {
				rawset(state, key, value);
			}

		public:
			object _metatable;
			std::unordered_map<object, object> data;
		};
	}
}

namespace lorelai {
	namespace vm {
		inline void referenceobject::concat(softwarestate &state, object &out, object &other) {
			out.set(stringobject::create(state, tostring(state) + other.tostring(state)));
		}
	}
}


#endif // OBJECT_HPP_
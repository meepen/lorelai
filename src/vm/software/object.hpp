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
		class softwarestate;
		using luafunction = int (*)(softwarestate &state, int nargs);
		class object;

		class referenceobject {
		public:
			virtual ~referenceobject() { }
			virtual _type _typeid() const = 0;

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
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(add,         softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(subtract,    softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(divide,      softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(multiply,    softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(power,       softwarestate &state, object &out, object &rhs);
			[[noreturn]] virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(modulo,      softwarestate &state, object &out, object &rhs);

			inline bool equals(softwarestate &state, object &other);

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

		class object {
		public:
			union multidata {
				number num;
				bool b;
				referenceobject *ref;
			};

		public:
			LORELAI_INLINE object(const object &obj) {
				type = obj.type;
				raw = obj.raw;
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
				type = STRING;
				raw.ref = &ref;
			}

			LORELAI_INLINE void set(const bool &b) {
				type = BOOL;
				raw.b = b;
			}

			LORELAI_INLINE void set(const number &num) {
				type = NUMBER;
				raw.num = num;
			}

			LORELAI_INLINE void set(const object &other) {
				type = other.type;
				raw = other.raw;
			}

			LORELAI_INLINE void set() {
				type = NIL;
			}

			LORELAI_INLINE void unset() {
				set();
			}

			// this is used for c++ structures and cannot be modified
			// easily done with reference comparisons
			bool operator==(const object &other) const {
				if (type != other.type) {
					return false;
				}

				switch (type) {
				case NUMBER:
					return raw.num == other.raw.num;
				case BOOL:
					return raw.b == other.raw.b;
				case NIL:
					return true;
				default:
					return raw.ref == other.raw.ref;
				}
			}

		public:
			// called in lua vm
			LORELAI_INLINE bool equals        (softwarestate &state, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (LORELAI_ISREFERENCETYPE(type | other.type)) {
					return raw.ref->equals(state, other);
				}

				// otherwise, check types
				// regular object can only be equal to the same type
				if (type != other.type) {
					return false;
				}

				switch (type) {
				case NUMBER:
					return raw.num == other.raw.num;
				case BOOL:
					return raw.b == other.raw.b;
				case NIL:
					return true;
				default:
					return false;
				}
			}

			LORELAI_INLINE bool lessthan      (softwarestate &state, object &other) {
				return tonumber(state) < other.tonumber(state);
			}
			LORELAI_INLINE bool greaterthan   (softwarestate &state, object &other) {
				return tonumber(state) > other.tonumber(state);
			}

			LORELAI_INLINE void add(softwarestate &state, object &out, object &other) {
				if ((type | other.type) == NUMBER) {
					out.set(raw.num + other.raw.num);
				}
				else if (LORELAI_ISREFERENCETYPE(type | other.type)) {
					throw;
				}
				else {
					out.set(tonumber(state) + other.tonumber(state));
				}
			}

			LORELAI_INLINE void subtract(softwarestate &state, object &out, object &other) {
				if ((type | other.type) == NUMBER) {
					out.set(raw.num - other.raw.num);
				}
				else if (LORELAI_ISREFERENCETYPE(type | other.type)) {
					throw;
				}
				else {
					out.set(tonumber(state) - other.tonumber(state));
				}
			}

			LORELAI_INLINE void divide(softwarestate &state, object &out, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type == NUMBER && other.type == NUMBER) {
					out.set(raw.num / other.raw.num);
				}
				else if (LORELAI_ISREFERENCETYPE(type | other.type)) {
					throw;
				}
				else {
					out.set(tonumber(state) / other.tonumber(state));
				}
			}

			LORELAI_INLINE void multiply(softwarestate &state, object &out, object &other) {
				if ((type | other.type) == NUMBER) {
					out.set(raw.num * other.raw.num);
				}
				else if (LORELAI_ISREFERENCETYPE(type | other.type)) {
					throw;
				}
				else {
					out.set(tonumber(state) * other.tonumber(state));
				}
			}

			LORELAI_INLINE void power(softwarestate &state, object &out, object &other) {
				if ((type | other.type) == NUMBER) {
					out.set(std::pow(raw.num, other.raw.num));
				}
				else if (LORELAI_ISREFERENCETYPE(type | other.type)) {
					throw;
				}
				else {
					out.set(std::pow(tonumber(state), other.tonumber(state)));
				}
			}

			LORELAI_INLINE void modulo(softwarestate &state, object &out, object &other) {
				if ((type | other.type) == NUMBER) {
					auto a = raw.num, b = other.raw.num;

					out.set(a - b * std::floor(a / b));
				}
				else if (LORELAI_ISREFERENCETYPE(type | other.type)) {
					throw;
				}
				else {
					auto a = tonumber(state), b = other.tonumber(state);

					out.set(a - b * std::floor(a / b));
				}
			}

			LORELAI_INLINE bool rawget(softwarestate &state, object &out, const object &index) {
				if (!LORELAI_ISREFERENCETYPE(type)) {
					throw exception(string("NYI: cannot index ") + gettypename());
				}

				return raw.ref->rawget(state, out, index);
			}
			LORELAI_INLINE void rawset(softwarestate &state, const object &index, const object &value) {
				if (!LORELAI_ISREFERENCETYPE(type)) {
					throw exception(string("NYI: cannot set index on ") + gettypename());
				}

				return raw.ref->rawset(state, index, value);
			}
			
			LORELAI_INLINE bool index            (softwarestate &state, object &out, object &index) {
				if (!LORELAI_ISREFERENCETYPE(type)) {
					throw exception(string("NYI: cannot index ") + gettypename());
				}

				return raw.ref->index(state, out, index);
			}
			
			LORELAI_INLINE void setindex            (softwarestate &state, object &key, object &value) {
				if (!LORELAI_ISREFERENCETYPE(type)) {
					throw exception(string("NYI: cannot setindex ") + gettypename());
				}

				return raw.ref->setindex(state, key, value);
			}

			LORELAI_INLINE state::_retdata call  (softwarestate &state, int nargs) {
				if (!LORELAI_ISREFERENCETYPE(type)) {
					throw exception(string("NYI: cannot call ") + gettypename());
				}

				return raw.ref->call(state, nargs);
			}

			[[noreturn]] LORELAI_INLINE void convertexception(const char *what) {
				throw exception(string("cannot convert ") + gettypename() + " to " + what);
			}

			LORELAI_INLINE number tonumber(softwarestate &state) {
				if (type == NUMBER) {
					return raw.num;
				}
				else if (LORELAI_ISREFERENCETYPE(type)) {
					return raw.ref->tonumber(state);
				}
				convertexception("number");
			}

			LORELAI_INLINE string tostring(softwarestate &state) const {
				switch (type) {
				case NIL:
					return "nil";
				case BOOL:
					return raw.b ? "true" : "false";
				case NUMBER:
					{
						std::ostringstream stream;
						stream.precision(13);
						stream << raw.num;
						return stream.str();
					}
				default:
					return raw.ref->tostring(state);
				}
			}

			LORELAI_INLINE bool tobool(softwarestate &state) {
				switch (type) {
					case NIL:
						return false;
					case BOOL:
						return raw.b;
					default:
						return true;
				}
			}

			LORELAI_INLINE object metatable(softwarestate &state) {
				if (LORELAI_ISREFERENCETYPE(type)) {
					return raw.ref->metatable(state);
				}

				return object();
			}

		public:
			LORELAI_INLINE const char *gettypename() {
				if (LORELAI_ISREFERENCETYPE(type)) {
					return typenames[raw.ref->_typeid()];
				}
				return typenames[type];
			}

		public:
			_type type = NIL;
			multidata raw;
		};
	}
}

namespace std {
	template<>
	struct hash<lorelai::vm::object> {
		size_t operator()(const lorelai::vm::object &obj) const {
			size_t r = std::hash<int>()(obj.type);
			switch (r) {
			case lorelai::vm::NUMBER:
				r ^= std::hash<lorelai::number>()(obj.raw.num);
				break;
			case lorelai::vm::BOOL:
				r ^= std::hash<bool>()(obj.raw.b);
			case lorelai::vm::NIL:
				break;
			default:
				r ^= obj.raw.ref->hash();
				break;
			}

			return r;
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


			_type _typeid() const override {
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
			_type _typeid() const override {
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
			_type _typeid() const override {
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
			_type _typeid() const override {
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
		inline bool referenceobject::equals(softwarestate &state, object &other) {
			return other.type == _typeid() && this == other.raw.ref;
		}

		inline void referenceobject::concat(softwarestate &state, object &out, object &other) {
			out.set(stringobject::create(state, tostring(state) + other.tostring(state)));
		}
	}
}


#endif // OBJECT_HPP_
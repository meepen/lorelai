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
#include "container.hpp"
#include <memory>
#include <unordered_map>

#define LORELAI_SOFTWARE_DEFAULT_FUNCTION(name, args...) name (args) { except(#name); }


namespace lorelai {
	namespace vm {
		class softwarestate;
		using luafunction = int (*)(softwarestate &state, int nargs, int nrets);
		class object;

		class referenceobject {
		protected:
			void except(string str) const { throw exception(string("cannot ")  + str + " a " + typenames[_typeid()]); }

		public:
			virtual ~referenceobject() { }
			virtual _type _typeid() const = 0;

			virtual size_t hash() const {
				return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(this));
			}

			virtual string tostring(softwarestate &state) {
				return typenames[_typeid()];
			}

			virtual bool            LORELAI_SOFTWARE_DEFAULT_FUNCTION(rawget,      softwarestate &state, object &out, const object &index)
			virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(rawset,      softwarestate &state, const object &index, const object &data)
			virtual state::_retdata LORELAI_SOFTWARE_DEFAULT_FUNCTION(call,        softwarestate &state, int nargs, int nrets)

			void LORELAI_SOFTWARE_DEFAULT_FUNCTION(length,      softwarestate &state, object &out)
			bool LORELAI_SOFTWARE_DEFAULT_FUNCTION(lessthan,    softwarestate &state, object &rhs);
			bool LORELAI_SOFTWARE_DEFAULT_FUNCTION(greaterthan, softwarestate &state, object &rhs);
			void LORELAI_SOFTWARE_DEFAULT_FUNCTION(add,         softwarestate &state, object &out, object &rhs);
			void LORELAI_SOFTWARE_DEFAULT_FUNCTION(subtract,    softwarestate &state, object &out, object &rhs);
			void LORELAI_SOFTWARE_DEFAULT_FUNCTION(divide,      softwarestate &state, object &out, object &rhs);
			void LORELAI_SOFTWARE_DEFAULT_FUNCTION(multiply,    softwarestate &state, object &out, object &rhs);
			void LORELAI_SOFTWARE_DEFAULT_FUNCTION(power,       softwarestate &state, object &out, object &rhs);
			void LORELAI_SOFTWARE_DEFAULT_FUNCTION(modulo,      softwarestate &state, object &out, object &rhs);

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
			union multidata {
				multidata() { }
				~multidata() { }

				string str;
				number num;
				bool b;
				software::container<referenceobject> ref;
			};

		public:
			~object() {
				if (type == STRING) {
					raw.str.~string();
				}
				else if (type >= TABLE) {
					raw.ref.~container();
				}
			}

			// copy constructor
			object(const object& other) {
				set(other);
			}

			// copy assignment
			object& operator=(const object& other)
			{
				return *this = object(other);
			}

			// move constructor
			object(object&& other) noexcept {
				set(other);
				other.unset();
			}

			// move assignment
			object& operator=(object&& other) noexcept 
			{
				set(other);
				other.unset();

				return *this;
			}

		public:
			object(const software::container<referenceobject> &ref, bool) {
				set(ref);
			}
			object(const number num) {
				set(num);
			}
			object(const string s) {
				set(s);
			}
			object(const char *s) {
				set(string(s));
			}
			object(const bool b) {
				set(b);
			}
			object() { }

		public:
			void settype(_type t) {
				if (type == t) {
					return;
				}

				if (type == STRING) {
					raw.str.~string();
				}
				else if (type >= TABLE) {
					raw.ref.~container();
				}

				type = t;

				if (type == STRING) {
					new (&raw.str) string();
				}
				else if (type >= TABLE) {
					new (&raw.ref) software::container<referenceobject>();
				}
			}

			inline void set(const string &str) {
				settype(STRING);
				raw.str = str;
			}

			inline void set(const bool &b) {
				settype(BOOL);
				raw.b = b;
			}

			inline void set(const number &num) {
				settype(NUMBER);
				raw.num = num;
			}

			inline void set(const object &other) {
				switch (other.type) {
				case NUMBER:
					set(other.raw.num);
					break;
				case STRING:
					set(other.raw.str);
					break;
				case BOOL:
					set(other.raw.b);
					break;
				case NIL:
					unset();
					break;
				default:
					set(other.raw.ref);
					break;
				}
			}

			inline void set(const software::container<referenceobject> &data) {
				if (!data.get()) {
					settype(NIL);
					return;
				}

				settype(data->_typeid());
				raw.ref = data;
			}

			inline void set() {
				unset();
			}

			inline void unset() {
				settype(NIL);
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
				case STRING:
					return raw.str == other.raw.str;
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
			inline bool equals        (softwarestate &state, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type >= TABLE) {
					return raw.ref->equals(state, other);
				}
				else if (other.type >= TABLE) {
					return other.raw.ref->equals(state, *this);
				}

				// otherwise, check types
				// regular object can only be equal to the same type
				if (type != other.type) {
					return false;
				}

				switch (type) {
				case NUMBER:
					return raw.num == other.raw.num;
				case STRING:
					return raw.str == other.raw.str;
				case BOOL:
					return raw.b == other.raw.b;
				case NIL:
					return true;
				default:
					return false;
				}
			}

			inline bool lessthan      (softwarestate &state, object &other) {
				return tonumber(state) < other.tonumber(state);
			}
			inline bool greaterthan   (softwarestate &state, object &other) {
				return tonumber(state) > other.tonumber(state);
			}

			inline void concat (softwarestate &state, object &out, object &other) {
				out.set(tostring(state) + other.tostring(state));
			}

			inline void add           (softwarestate &state, object &out, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type == NUMBER && other.type == NUMBER) {
					out.set(raw.num + other.raw.num);
				}
				else if (type >= TABLE) {
					return raw.ref->add(state, out, other);
				}
				else if (other.type >= TABLE) {
					return other.raw.ref->add(state, out, *this);
				}
				else {
					out.set(tonumber(state) + other.tonumber(state));
				}
			}

			inline void subtract      (softwarestate &state, object &out, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type == NUMBER && other.type == NUMBER) {
					out.set(raw.num - other.raw.num);
				}
				else if (type >= TABLE) {
					return raw.ref->subtract(state, out, other);
				}
				else if (other.type >= TABLE) {
					return other.raw.ref->subtract(state, out, *this);
				}
				else {
					out.set(tonumber(state) - other.tonumber(state));
				}
			}

			inline void divide        (softwarestate &state, object &out, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type == NUMBER && other.type == NUMBER) {
					out.set(raw.num / other.raw.num);
				}
				else if (type >= TABLE) {
					return raw.ref->divide(state, out, other);
				}
				else if (other.type >= TABLE) {
					return other.raw.ref->divide(state, out, *this);
				}
				else {
					out.set(tonumber(state) / other.tonumber(state));
				}
			}

			inline void multiply      (softwarestate &state, object &out, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type == NUMBER && other.type == NUMBER) {
					out.set(raw.num * other.raw.num);
				}
				else if (type >= TABLE) {
					return raw.ref->multiply(state, out, other);
				}
				else if (other.type >= TABLE) {
					return other.raw.ref->multiply(state, out, *this);
				}
				else {
					out.set(tonumber(state) * other.tonumber(state));
				}
			}

			inline void power         (softwarestate &state, object &out, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type == NUMBER && other.type == NUMBER) {
					out.set(std::pow(raw.num, other.raw.num));
				}
				else if (type >= TABLE) {
					return raw.ref->power(state, out, other);
				}
				else if (other.type >= TABLE) {
					return other.raw.ref->power(state, out, *this);
				}
				else {
					out.set(std::pow(tonumber(state), other.tonumber(state)));
				}
			}

			inline void modulo        (softwarestate &state, object &out, object &other) {
				// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
				if (type == NUMBER && other.type == NUMBER) {
					out.set(raw.num - other.raw.num * std::floor(raw.num / other.raw.num));
				}
				else if (type >= TABLE) {
					return raw.ref->modulo(state, out, other);
				}
				else if (other.type >= TABLE) {
					return other.raw.ref->modulo(state, out, *this);
				}
				else {
					auto a = tonumber(state), b = other.tonumber(state);

					out.set(a - b * std::floor(a / b));
				}
			}

			virtual bool rawget(softwarestate &state, object &out, const object &index) {
				if (type < TABLE) {
					throw exception(string("NYI: cannot index ") + gettypename());
				}

				return raw.ref->rawget(state, out, index);
			}
			virtual void rawset(softwarestate &state, const object &index, const object &value) {
				if (type < TABLE) {
					throw exception(string("NYI: cannot set index on ") + gettypename());
				}

				return raw.ref->rawset(state, index, value);
			}
			
			bool index            (softwarestate &state, object &out, object &index) {
				if (type < TABLE) {
					throw exception(string("NYI: cannot index ") + gettypename());
				}

				return raw.ref->index(state, out, index);
			}
			state::_retdata call  (softwarestate &state, int nargs, int nrets) {
				if (type < TABLE) {
					throw exception(string("NYI: cannot call ") + gettypename());
				}

				return raw.ref->call(state, nargs, nrets);
			}

			inline number tonumber    (softwarestate &state) {
				if (type == NUMBER) {
					return raw.num;
				}
				else if (type == STRING) {
					return lorelai::tonumber(raw.str);
				}
				
				throw exception(string("cannot convert ") + gettypename() + " to number");
			}

			inline string tostring    (softwarestate &state) {
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
				case STRING:
					return raw.str;

				default:
					throw exception(string("cannot convert ") + gettypename() + " to string");
				}
			}

			inline bool   tobool      (softwarestate &state) {
				switch (type) {
					case NIL:
						return false;
					case BOOL:
						return raw.b;
					default:
						return true;
				}
			}

			object metatable(softwarestate &state) {
				if (type >= TABLE) {
					return raw.ref->metatable(state);
				}

				return object();
			}

		public:
			const char *gettypename() {
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
			size_t r;

			if (obj.type >= lorelai::vm::TABLE) {
				r = obj.raw.ref->hash();
			}
			else {
				r = std::hash<int>()(obj.type);
				switch (r) {

				case lorelai::vm::NUMBER:
					r ^= std::hash<lorelai::number>()(obj.raw.num);
					break;

				case lorelai::vm::STRING:
					r ^= std::hash<lorelai::string>()(obj.raw.str);
					break;

				case lorelai::vm::BOOL:
					r ^= std::hash<bool>()(obj.raw.b);

				case lorelai::vm::NIL:
				default:
					break;
				}
			}

			return r;
		}
	};
}

namespace lorelai {
	namespace vm {
		class functionobject : public referenceobject {
		protected:
			functionobject() { }

		public:
			object metatable(softwarestate &state) const override;
		};

		class luafunctionobject : public functionobject {
		public:
			static object create(softwarestate &state, std::shared_ptr<bytecode::prototype> proto);
			static void init();

		public:
			luafunctionobject(softwarestate &state, std::shared_ptr<bytecode::prototype> proto);
			luafunctionobject();
			~luafunctionobject();

		public:
			_type _typeid() const override {
				return LUAFUNCTION;
			}

			state::_retdata call(softwarestate &state, int nargs, int nrets) override;

		public:
			struct instruction;
			std::shared_ptr<instruction> allocated;
			std::uint32_t stacksize;
			std::vector<object> strings;
			std::vector<object> numbers;
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

			state::_retdata call(softwarestate &state, int nargs, int nrets) override;

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
				}
				else {
					out = found->second;
				}
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
			return other.type == _typeid() && this == other.raw.ref.get();
		}

		inline void referenceobject::concat(softwarestate &state, object &out, object &other) {
			out.set(tostring(state) + other.tostring(state));
		}
	}
}


#endif // OBJECT_HPP_
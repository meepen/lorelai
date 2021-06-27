#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include <cstdint>
#include <memory>
#include <sstream>

#include "software.hpp"
#include "types.hpp"
#include "bytecode.hpp"
#include "container.hpp"
#include <memory>
#include <unordered_map>

#define LORELAI_SOFTWARE_DEFAULT_FUNCTION(name, args...) name (args) { except(#name); }

namespace lorelai {
	namespace vm {
		using luafunction = int (*)(softwarestate &state, int nrets, int nargs);

		class object {
		protected:
			void except(string str) const { throw exception(string("cannot ")  + str + " a " + type()); }
			enum _type {
				NIL,
				BOOL,
				LUAFUNCTION,
				CFUNCTION,
				STRING,
				NUMBER,
				TABLE
			};

		public:
			virtual ~object() { }
			virtual const char *type() const = 0;
			virtual _type _typeid() const = 0;

			virtual size_t hash() const {
				return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(this));
			}

			virtual string tostring(softwarestate &state) {
				return type();
			}

			virtual bool tobool(softwarestate &state) {
				return true;
			}

			virtual number          LORELAI_SOFTWARE_DEFAULT_FUNCTION(tonumber,    softwarestate &state)
			virtual bool            LORELAI_SOFTWARE_DEFAULT_FUNCTION(rawget,      softwarestate &state, objectcontainer &out, objectcontainer rhs)
			virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(rawset,      softwarestate &state, objectcontainer lhs, objectcontainer rhs)
			virtual void            LORELAI_SOFTWARE_DEFAULT_FUNCTION(length,      softwarestate &state, objectcontainer &out, objectcontainer &obj)
			virtual state::_retdata LORELAI_SOFTWARE_DEFAULT_FUNCTION(call,        softwarestate &state, int nrets, int nargs)

			virtual bool equals      (const object &b) const { return this == &b; }

			bool equals      (softwarestate &state, objectcontainer rhs) { return equals(*rhs.get()); }
			bool lessthan    (softwarestate &state, objectcontainer rhs);
			bool greaterthan (softwarestate &state, objectcontainer rhs);

			void concat        (softwarestate &state, objectcontainer &out, objectcontainer rhs);
			void add           (softwarestate &state, objectcontainer &out, objectcontainer rhs);
			void subtract      (softwarestate &state, objectcontainer &out, objectcontainer rhs);
			void divide        (softwarestate &state, objectcontainer &out, objectcontainer rhs);
			void multiply      (softwarestate &state, objectcontainer &out, objectcontainer rhs);
			void power         (softwarestate &state, objectcontainer &out, objectcontainer rhs);
			void modulo        (softwarestate &state, objectcontainer &out, objectcontainer rhs);

			virtual bool index(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
				if (!rawget(state, out, rhs)) {
					// TODO: metatable access
					auto mt = metatable(state);

					return false;
				}

				return true;
			}

			virtual objectcontainer metatable(softwarestate &state) const = 0;

		public:
			struct equals_to {
				bool operator()(const objectcontainer &obj1, const objectcontainer &obj2) const {
					return obj1->equals(*obj2.get());
				}
			};
		};
	}
}


namespace std {
	template<> struct hash<lorelai::vm::object *> {
		size_t operator()(const lorelai::vm::object *const &obj) {
			return obj->hash();
		}
	};
}

namespace lorelai {
	namespace vm {
		class nilobject : public object {
		public:
			static objectcontainer create(softwarestate &state);
			const char *type() const override { return "nil"; }
			bool equals(const object & b) const override {
				return this == &b;
			}
			size_t hash() const override {
				return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(this));
			}

			_type _typeid() const override {
				return NIL;
			}

			objectcontainer metatable(softwarestate &state) const override {
				return state.nil_metatable;
			}
			bool tobool(softwarestate &state) override {
				return false;
			}

		private:
			static objectcontainer nil_metatable;
		};

		class numberobject : public object {
		public:
			static objectcontainer create(softwarestate &state, number n);
			numberobject(number num) : data(num) { }
			numberobject() : data(0.0) { }
		public:
			const char *type() const override { return "number"; }
			_type _typeid() const override {
				return NUMBER;
			}

			size_t hash() const override {
				return std::hash<number>()(data);
			}
			bool equals(const object & b) const override {
				return _typeid() == b._typeid() && dynamic_cast<const numberobject &>(b).data == data;
			}
			objectcontainer metatable(softwarestate &state) const override {
				return state.number_metatable;
			}

			string tostring(softwarestate &state) override {
				std::ostringstream stream;
				stream.precision(13);
				stream << data;
				return stream.str();
			}

			number tonumber(softwarestate &state) override {
				return data;
			}

		public:
			number data = 0;

		private:
			static objectcontainer number_metatable;
		};

		class boolobject : public object {
		public:
			static objectcontainer create(softwarestate &state, bool b);
			boolobject(bool b) : data(b) { }
			boolobject() : data(false) { }

		public:
			const char *type() const override { return "boolean"; }
			_type _typeid() const override {
				return BOOL;
			}

			size_t hash() const override {
				return std::hash<bool>()(data);
			}

			bool equals(const object & b) const override {
				return _typeid() == b._typeid() && dynamic_cast<const boolobject &>(b).data == data;
			}
			bool tobool(softwarestate &state) override {
				return data;
			}
			objectcontainer metatable(softwarestate &state) const override {
				return state.boolean_metatable;
			}
			string tostring(softwarestate &state) override {
				return data ? "true" : "false";
			}

		public:
			bool data;

		private:
			static objectcontainer boolean_metatable;
		};

		class stringobject : public object {
		public:
			static objectcontainer create(softwarestate &state, string s);
			stringobject(string str) : data(str) { }
			stringobject() : data("") { }
		public:
			const char *type() const override { return "string"; }
			_type _typeid() const override {
				return STRING;
			}

			bool equals(const object & b) const override {
				return _typeid() == b._typeid() && dynamic_cast<const stringobject &>(b).data == data;
			}

			objectcontainer metatable(softwarestate &state) const override {
				return state.string_metatable;
			}

			size_t hash() const override {
				return std::hash<string>()(data);
			}

			string tostring(softwarestate &state) override {
				return data;
			}

			number tonumber(softwarestate &state) override {
				return lorelai::tonumber(data);
			}

		public:
			string data = "";

		private:
			static objectcontainer string_metatable;
		};

		extern objectcontainer function_metatable;
		class functionobject : public object {
		protected:
			functionobject() { }
		public:
			const char *type() const override { return "function"; }

			bool equals(const object & b) const override {
				return &b == this;
			}

			objectcontainer metatable(softwarestate &state) const override {
				return state.function_metatable;
			}
		};

		class luafunctionobject : public functionobject {
		public:
			_type _typeid() const override {
				return LUAFUNCTION;
			}
			static objectcontainer create(softwarestate &state, std::shared_ptr<bytecode::prototype> proto);

			luafunctionobject(std::shared_ptr<bytecode::prototype> proto) : data(proto) { }
			luafunctionobject() { }
			
			state::_retdata call(softwarestate &state, int nrets, int nargs) override;
		public:
			std::shared_ptr<bytecode::prototype> data;
		};

		class cfunctionobject : public functionobject {
		public:
			static objectcontainer create(softwarestate &state, luafunction func);
			cfunctionobject(luafunction func) : data(func) { }
			cfunctionobject() { }
		public:
			const char *type() const override { return "cfunction"; }
			_type _typeid() const override {
				return CFUNCTION;
			}

			bool equals(const object & b) const override {
				return &b == this;
			}

			state::_retdata call(softwarestate &state, int nrets, int nargs) override;
		public:
			luafunction data;
		};

		class tableobject : public object {
		public:
			tableobject() { }
			static objectcontainer create(softwarestate &state);

			const char *type() const override { return "table"; }
			_type _typeid() const override {
				return TABLE;
			}

			objectcontainer metatable(softwarestate &state) const override {
				return _metatable;
			}
			void rawset(softwarestate &state, objectcontainer lhs, objectcontainer rhs) override {
				data[lhs] = rhs;
			}
			bool rawget(softwarestate &state, objectcontainer &out, objectcontainer index) override {
				auto found = data.find(index);
				if (found == data.end()) {
					out = nilobject::create(state);
				}
				else {
					out = found->second;
				}
			}
		public:
			objectcontainer _metatable = nullptr;
			std::unordered_map<objectcontainer, objectcontainer, std::hash<objectcontainer>, object::equals_to> data;
		};
	}
}

#endif // OBJECT_HPP_
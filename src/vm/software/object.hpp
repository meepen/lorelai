#ifndef JIT_TYPES_HPP_
#define JIT_TYPES_HPP_

#include <cstdint>
#include <memory>
#include <sstream>

#include "state.hpp"
#include "types.hpp"
#include "bytecode.hpp"
#include <memory>
#include <unordered_map>

#define LORELAI_SOFTWARE_DEFAULT_FUNCTION(args...) (args) { except(); }

namespace lorelai {
	namespace vm {
		struct object;
		class softwarestate;
		using objectcontainer = std::shared_ptr<object>;
		using luafunction = int (*)(softwarestate &state, int nrets, int nargs);

		class object {
		protected:
			void except() const { throw; }

		public:
			virtual ~object() { }
			virtual const char *type() const = 0;
			virtual bool equals(const object &b) const { return this == &b; }

			virtual size_t hash() const {
				return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(this));
			}

			virtual string tostring(softwarestate &state) {
				return type();
			}

			virtual void add                   LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void sub                   LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void div                   LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void mul                   LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void pow                   LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void mod                   LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void lessthan              LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void greaterthan           LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual void concat                LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs)
			virtual bool rawget                LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer rhs)
			virtual void rawset                LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer lhs, objectcontainer rhs)
			virtual void len                   LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, objectcontainer &out, objectcontainer &obj)
			virtual state::_retdata  call      LORELAI_SOFTWARE_DEFAULT_FUNCTION(softwarestate &state, int nrets, int nargs)
			virtual bool index(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
				if (!rawget(state, out, rhs)) {
					// TODO: metatable access
					auto mt = metatable();

					return false;
				}

				return true;
			}

			virtual std::shared_ptr<object> metatable() const = 0;

		public:
			struct equals_to {
				bool operator()(const std::shared_ptr<lorelai::vm::object> &obj1, const std::shared_ptr<lorelai::vm::object> &obj2) const {
					return obj1->equals(*obj2);
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
			const char *type() const override { return "nil"; }
			bool equals(const object & b) const override {
				return metatable() == b.metatable();
			}
			size_t hash() const override {
				return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(&nil_metatable));
			}

			std::shared_ptr<object> metatable() const override {
				return nil_metatable;
			}

		private:
			static std::shared_ptr<object> nil_metatable;
		};

		class numberobject : public object {
		public:
			numberobject(number num) : data(num) { }
		public:
			const char *type() const override { return "number"; }
			bool equals(const object & b) const override {
				return metatable() == b.metatable() && dynamic_cast<const numberobject &>(b).data == data;
			}
			std::shared_ptr<object> metatable()const  override {
				return number_metatable;
			}
			size_t hash() const override {
				return std::hash<number>()(data);
			}

			string tostring(softwarestate &state) override {
				std::ostringstream stream;
				stream.precision(13);
				stream << data;
				return stream.str();
			}

		public:
			number data = 0;

		private:
			static std::shared_ptr<object> number_metatable;
		};

		class stringobject : public object {
		public:
			stringobject(string str) : data(str) { }
		public:
			const char *type() const override { return "string"; }
			bool equals(const object & b) const override {
				return metatable() == b.metatable() && dynamic_cast<const stringobject &>(b).data == data;
			}

			std::shared_ptr<object> metatable() const override {
				return string_metatable;
			}

			size_t hash() const override {
				return std::hash<string>()(data);
			}

			string tostring(softwarestate &state) override {
				return data;
			}

		public:
			string data = "";

		private:
			static std::shared_ptr<object> string_metatable;
		};

		extern std::shared_ptr<object> function_metatable;
		class functionobject : public object {
		protected:
			functionobject() { }
		public:
			const char *type() const override { return "function"; }
			bool equals(const object & b) const override {
				return &b == this;
			}

			std::shared_ptr<object> metatable() const override {
				return function_metatable;
			}
		};

		class luafunctionobject : public functionobject {
		public:
			luafunctionobject(std::shared_ptr<bytecode::prototype> proto) : data(proto) { }
			
			state::_retdata call(softwarestate &state, int nrets, int nargs) override;
		public:
			std::shared_ptr<bytecode::prototype> data;
		};

		class cfunctionobject : public functionobject {
		public:
			cfunctionobject(luafunction func) : data(func) { }
		public:
			const char *type() const override { return "function"; }
			bool equals(const object & b) const override {
				return &b == this;
			}

			state::_retdata call(softwarestate &state, int nrets, int nargs) override;
		public:
			luafunction data;
		};

		class tableobject : public object {
		public:
			const char *type() const override { return "table"; }

			std::shared_ptr<object> metatable() const override {
				return _metatable;
			}
			void rawset(softwarestate &state, objectcontainer lhs, objectcontainer rhs) override {
				data[lhs] = rhs;
			}
			bool rawget(softwarestate &state, objectcontainer &out, objectcontainer index) override {
				auto found = data.find(index);
				if (found == data.end()) {
					out = std::make_shared<nilobject>();
				}
				else {
					out = found->second;
				}
			}
		public:
			std::shared_ptr<object> _metatable = nullptr;
			std::unordered_map<objectcontainer, objectcontainer, std::hash<objectcontainer>, object::equals_to> data;
		};
	}
}

#endif // JIT_TYPES_HPP_
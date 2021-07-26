#ifndef LORELAI_PROTOTYPE_HPP_
#define LORELAI_PROTOTYPE_HPP_

#include "bytebuffer.hpp"
#include "types.hpp"

#define LORELAI_OPCODES(fn) \
	fn(CONSTANT)         /* A = ({true, false, nil})[B] */ \
	fn(MOV)              /* A .. A+C = B .. B+C */ \
	fn(ADD)              /* A = B + C */ \
	fn(NOT)              /* A = not B */ \
	fn(RETURN)           /* return A .. A+B-1, C == 1 and ... or C == 2 and returns... or none */ \
	fn(CALL)             /* A .. A+C-2 = A(A+1 .. A + B) */ \
	fn(CALLV)            /* A .. A+C-2 = A(A+1 .. A + B, ...) */ \
	fn(UPVALSET)         /* UP(A)[B] = C */ \
	fn(NEWFUNC)          /* A = NEWFUNC(prototypes[B], C...) */ \
	fn(STRING)           /* A = string(B) */ \
	fn(NUMBER)           /* A = number(B) */ \
	fn(FUNCTION)         /* A = function(B) */ \
	fn(TABLE)            /* A = table(B) */ \
	fn(SUBTRACT)         /* A = B - C */ \
	fn(DIVIDE)           /* A = B / C */ \
	fn(MULTIPLY)         /* A = B * C */ \
	fn(POWER)            /* A = B ^ C */ \
	fn(MODULO)           /* A = B % C */ \
	fn(CONCAT)           /* A = B .. C */ \
	fn(INDEX)            /* A = B [ C ] */ \
	fn(OR)               /* A = B or C */ \
	fn(AND)              /* A = B and C */ \
	fn(LESSTHAN)         /* A = B < C */ \
	fn(LESSTHANEQUAL)    /* A = B <= C */ \
	fn(GREATERTHAN)      /* A = B > C */ \
	fn(GREATERTHANEQUAL) /* A = B >= C */ \
	fn(EQUALS)           /* A = B == C */ \
	fn(NOTEQUALS)        /* A = B ~= C */ \
	fn(MINUS)            /* A = -B */ \
	fn(LENGTH)           /* A = #B */ \
	fn(ENVIRONMENT)      /* A = _ENV */ \
	fn(SETINDEX)         /* A[B] = C */ \
	fn(ENVIRONMENTGET)   /* A = _ENV[str(B)] */ \
	fn(ENVIRONMENTSET)   /* _ENV[str(A)] = B */ \
	fn(JMP)              /* JMP(instruction(B)) */ \
	fn(JMPIFTRUE)        /* if (A) { JMP(instruction(B)) } */ \
	fn(JMPIFFALSE)       /* if (!A) { JMP(instruction(B)) } */ \
	fn(JMPIFNIL)         /* if (A == nil) { JMP(instruction(B)) } */ \
	fn(CALLM)            /* A .. A+C-2 = A(A+1 .. A + B, returns...) */ \
	fn(VARARG)           /* A .. A+C = ... */ \
	fn(FORCHECK)         /* if (!((A + 2) > 0 ? (A) <= (A + 1) : (A + 2) <= 0 && (A) >= (A + 1)) { JMP(instruction(B)) } */ \
	fn(FNEW)             /* A = function(B) */ \
	fn(MOV1)             /* A = B */ \
	fn(MAX)


namespace lorelai {
	namespace bytecode {

		class prototype {
		public:
			enum _opcode {
#define LORELAI_OP(x) OP_##x,
				LORELAI_OPCODES(LORELAI_OP)
#undef LORELAI_OP
			};

			struct _tablevalue {
				enum _valuetype {
					CONSTANT = 0,
					NUMBER = 1,
					STRING = 2,
					TABLE = 3,
					FUNCTION = 4,
				};
				_valuetype type = CONSTANT;
				std::uint32_t index = 0;


				// protobuf

				_tablevalue &set_type(_valuetype _type) {
					type = _type;

					return *this;
				}
				_tablevalue &set_index(std::uint32_t _idx) {
					index = _idx;

					return *this;
				}
			};

			struct _hashpartvalue {
				_tablevalue key;
				_tablevalue value;
			};

			struct _table {
				std::vector<_tablevalue> arraypart;
				std::vector<_hashpartvalue> hashpart;

				_tablevalue *add_arraypart() {
					arraypart.emplace_back();
					return &arraypart.back();
				}
				const _tablevalue &arrayparts(size_t i) const {
					return arraypart[i];
				}

				_hashpartvalue *add_hashpart() {
					hashpart.emplace_back();
					return &hashpart.back();
				}
				const _hashpartvalue &hashparts(size_t i) const {
					return hashpart[i];
				}
			};

			struct _upvaluereference {
				std::uint32_t stackindex;
			};

			struct instruct {
				instruct() { }
				instruct(prototype::_opcode _op, std::uint32_t _a = 0, std::uint32_t _b = 0, std::uint32_t _c = 0) {
					set_op(_op);
					set_a(_a);
					set_b(_b);
					set_c(_c);
				}

				std::uint32_t &op() {
					return fields[0];
				}
				std::uint32_t &a() {
					return fields[1];
				}
				std::uint32_t &b() {
					return fields[2];
				}
				std::uint32_t &c() {
					return fields[3];
				}

				instruct &set_op(const _opcode &v) {
					op() = static_cast<const std::uint32_t>(v);

					return *this;
				}
				instruct &set_a(const std::uint32_t &v) {
					a() = v;

					return *this;
				}
				instruct &set_b(const std::uint32_t &v) {
					b() = v;

					return *this;
				}
				instruct &set_c(const std::uint32_t &v) {
					c() = v;

					return *this;
				}
				instruct &set_bc(const std::uint64_t &v) {
					std::memcpy(&fields[2], &v, sizeof(v));

					return *this;
				}
				instruct &set_bcnum(const number &v) {
					std::uint64_t n;
					std::memcpy(&n, &v, sizeof(v));
					return set_bc(n);
				}
				std::uint64_t bc() {
					std::uint64_t _bc;
					std::memcpy(&_bc, &fields[2], sizeof(_bc));
					return _bc;
				}
				number bcnum() {
					auto _bc = bc();
					lorelai::number num;
					std::memcpy(&num, &_bc, sizeof(num));
					return num;
				}


				instruct *operator->() {
					return this;
				}

				std::uint32_t fields[4];
			};

			static_assert(sizeof(instruct) == 16);

			struct instruct_ptr {
				prototype *proto;
				size_t index;

				instruct *operator->() {
					return &proto->instructions[index];
				}
			};

		public:
			prototype() { }

		public:
			instruct_ptr addinstruction(_opcode opcode, std::uint32_t a = 0, std::uint32_t b = 0, std::uint32_t c = 0) {
				instruct_ptr ins = { this, instructions.size() };
				instructions.emplace_back(opcode, a, b, c);
				
				return ins;
			}

			size_t instructions_size() const {
				return instructions.size();
			}
			instruct &instruction(size_t i) {
				return instructions[i];
			}

		public:
			size_t tables_size() const {
				return tables.size();
			}
			_table *add_tables() {
				tables.emplace_back();
				return &tables.back();
			}
			const _table &table(size_t i) const {
				return tables[i];
			}

			size_t strings_size() const {
				return strings.size();
			}
			std::string *add_strings() {
				strings.emplace_back();
				return &strings.back();
			}
			const std::string &string(size_t i) const {
				return strings[i];
			}

			size_t upvalues_size() const {
				return upvalues.size();
			}
			_upvaluereference *add_upvalues() {
				upvalues.emplace_back();
				return &upvalues.back();
			}
			const _upvaluereference &upvalue(size_t i) const {
				return upvalues[i];
			}

			size_t numbers_size() const {
				return numbers.size();
			}
			double *add_numbers() {
				numbers.emplace_back();
				return &numbers.back();
			}
			const double &number(size_t i) const {
				return numbers[i];
			}

			size_t protos_size() const {
				return protos.size();
			}
			prototype *add_protos() {
				protos.emplace_back();
				return &protos.back();
			}
			prototype &proto(size_t i) {
				return protos[i];
			}

		public:
			std::vector<instruct> instructions;
			std::vector<double> numbers;
			std::vector<std::string> strings;
			std::vector<_table> tables;
			std::vector<prototype> protos;
			std::vector<_upvaluereference> upvalues;
			std::uint32_t stacksize = 8;

			std::string identifier = "<unknown>";
		};

		static const char *opcode_names[prototype::_opcode::OP_MAX + 1] = {
#define LORELAI_OPNAME(x) #x,
			LORELAI_OPCODES(LORELAI_OPNAME)
#undef LORELAI_OPNAME
		};

		template <>
		struct write<prototype::_upvaluereference> {
			void operator()(const prototype::_upvaluereference &t, writebuffer &buffer) const {
				buffer.write(t.stackindex);
			}
		};
		template <>
		struct read<prototype::_upvaluereference> {
			void operator()(prototype::_upvaluereference &out, readbuffer &buffer) const {
				buffer.read(out.stackindex);
			}
		};

		template <>
		struct write<prototype::_tablevalue> {
			void operator()(const prototype::_tablevalue &t, writebuffer &buffer) const {
				buffer.write<std::uint8_t>(t.type);
				buffer.write(t.index);
			}
		};
		template <>
		struct read<prototype::_tablevalue> {
			void operator()(prototype::_tablevalue &out, readbuffer &buffer) const {
				out.type = static_cast<prototype::_tablevalue::_valuetype>(buffer.read<std::uint8_t>());
				buffer.read(out.index);
			}
		};

		template <>
		struct write<prototype::_hashpartvalue> {
			void operator()(const prototype::_hashpartvalue &t, writebuffer &buffer) const {
				buffer.write(t.key);
				buffer.write(t.value);
			}
		};
		template <>
		struct read<prototype::_hashpartvalue> {
			void operator()(prototype::_hashpartvalue &out, readbuffer &buffer) const {
				buffer.read(out.key);
				buffer.read(out.value);
			}
		};

		template <>
		struct write<prototype::_table> {
			void operator()(const prototype::_table &t, writebuffer &buffer) const {
				buffer.write(t.arraypart);
				buffer.write(t.hashpart);
			}
		};
		template <>
		struct read<prototype::_table> {
			void operator()(prototype::_table &out, readbuffer &buffer) const {
				buffer.read(out.arraypart);
				buffer.read(out.hashpart);
			}
		};

		template <>
		struct write<prototype::instruct> {
			void operator()(const prototype::instruct &t, writebuffer &buffer) const {
				buffer.write(t.fields[0]);
				buffer.write(t.fields[1]);
				buffer.write(t.fields[2]);
				buffer.write(t.fields[3]);
			}
		};
		template <>
		struct read<prototype::instruct> {
			void operator()(prototype::instruct &out, readbuffer &buffer) const {
				buffer.read(out.fields[0]);
				buffer.read(out.fields[1]);
				buffer.read(out.fields[2]);
				buffer.read(out.fields[3]);
			}
		};

		template <>
		struct write<prototype> {
			void operator()(const prototype &p, writebuffer &buffer) const {
				buffer.write(p.instructions);

				buffer.write(p.numbers);
				buffer.write(p.strings);
				buffer.write(p.tables);
				buffer.write(p.protos);
				buffer.write(p.upvalues);

				buffer.write(p.stacksize);
				buffer.write(p.identifier);
			}
		};
		template <>
		struct read<prototype> {
			void operator()(prototype &out, readbuffer &buffer) const {
				out.instructions = buffer.read<std::vector<prototype::instruct>>();
				out.numbers = buffer.read<std::vector<double>>();
				out.strings = buffer.read<std::vector<std::string>>();
				out.tables = buffer.read<std::vector<prototype::_table>>();
				out.protos = buffer.read<std::vector<prototype>>();
				out.upvalues = buffer.read<std::vector<prototype::_upvaluereference>>();

				out.stacksize = buffer.read<std::uint32_t>();
				out.identifier = buffer.read<std::string>();
			}
		};
	}
}

#endif // LORELAI_PROTOTYPE_HPP_
#include "../object.hpp"
#include "../software.hpp"
#include <exception>
#include <string>
#include <memory>
#include <iostream>

using namespace lorelai;
using namespace lorelai::vm;
using namespace lorelai::bytecode;
#define vmcase(x) case prototype::OP_##x:
#define vmbreak goto instructionstart

void luafunctionobject::fromtablevalue(object &out, const prototype::_tablevalue &data) {
	switch (data.type) {
	case prototype::_tablevalue::CONSTANT:
		switch (data.index) {
		case 0:
			out.set(true);
			break;
		case 1:
			out.set(false);
			break;
		default:
			out.set();
			break;
		}
		break;

	case prototype::_tablevalue::NUMBER:
		out.set(numbers[data.index]);
		break;

	case prototype::_tablevalue::STRING:
		out.set(strings[data.index]);
		break;

	default:
		throw exception("unsupported type in table");
	}
}


state::_retdata luafunctionobject::call(softwarestate &state, int nargs) {
	int multres = 0;
	state::_retdata retdata {0, 0};

	state->stacktop = state->stackptr + stacksize;
	auto starts = allocated.get();

	instruction *next = starts;
	instruction *instr;

	instructionstart:
	instr = next++;
	switch (instr->op()) {
		vmcase(RETURN)
			return retdata;
		vmcase (JMPIFTRUE)
			if (state[instr->a()].tobool(state)) {
				next = starts + instr->b();
			}
			vmbreak;
		vmcase (JMPIFFALSE)
			if (!state[instr->a()].tobool(state)) {
				next = starts + instr->b();
			}
			vmbreak;
		vmcase (FORCHECK) {
			auto a = instr->a();
			auto start = state[a].tonumber(state);
			auto ends = state[a + 1].tonumber(state);
			auto step = state[a + 2].tonumber(state);

			if (!(step > 0 ? start <= ends : step <= 0 && start >= ends)) {
				next = starts + instr->b();
			}
			vmbreak;
		}
		vmcase (JMP)
			next = starts + instr->b();
			vmbreak;
		vmcase (CONSTANT) {
			switch (instr->b()) {
			case 0:
				state[instr->a()].set(true);
				vmbreak;
			case 1:
				state[instr->a()].set(false);
				vmbreak;
			default:
				state[instr->a()].unset();
				vmbreak;
			}
			vmbreak;
		}
		vmcase (MOV) {
			std::uint8_t a = instr->a(), b = instr->b(), c = instr->c() + 1;

			while (c--) {
				state[a++].set(state[b++]);
			}
			vmbreak;
		}
		vmcase (MOV1)
			state[instr->a()].set(state[instr->b()]);
			vmbreak;
		/*vmcase (CALLM) {
			// A .. A+C-2 = A(A+1 .. A + B, ...)
			for (int i = multres - 1; i >= 0; i--) {
				state->stacktop[i + instr->b() + 1].set(state->stacktop[i]);
			}
			for (unsigned i = 0; i <= instr->b(); i++) {
				state->stacktop[i].set(state[instr->a() + i]);
			}

			state[0].call(state, instr->b() + multres, instr->c() - 1);
			if (instr->c() >= 1) {
				// x res
			}
			else {
				// multres
			}
			vmbreak;
		}*/
		vmcase (CALL) {
			// A .. A+C-1, ... = A(A+1 .. A + B)
			for (int i = 0; i <= instr->b(); i++) {
				state->stacktop[i].set(state[instr->a() + i]);
			}
			auto old = state->pushstack(instr->b() + 1);
			auto retdata = state[0].call(state, instr->b());
			auto nrets = state->popstack(old, retdata);

			for (int i = 0; i < instr->c(); i++) {
				state[instr->a() + i].set(state->stacktop[i]);
			}

			multres = nrets - instr->c();
			for (int i = instr->c(); i < nrets; i++) {
				state->stacktop[i].set(state->stacktop[i + instr->c()]);
			}

			vmbreak;
		}
		vmcase (MINUS)
			state[instr->a()].set(-state[instr->b()].tonumber(state));
			vmbreak;
		vmcase (ENVIRONMENTGET)
			state.registry.index(state, state[instr->a()], strings[instr->b()]);
			vmbreak;
		vmcase (STRING)
			state[instr->a()].set(strings[instr->b()]);
			vmbreak;
		vmcase (NUMBER)
			state[instr->a()].set(instr->bcnum());
			vmbreak;
		vmcase (INDEX)
			state[instr->b()].index(state, state[instr->a()], state[instr->c()]);
			vmbreak;
		vmcase (SETINDEX)
			state[instr->a()].setindex(state, state[instr->b()], state[instr->c()]);
			vmbreak;
		vmcase (NOT)
			state[instr->a()].set(!state[instr->b()].tobool(state));
			vmbreak;
		vmcase (NOTEQUALS)
			state[instr->a()].set(!state[instr->c()].equals(state, state[instr->b()]));
			vmbreak;
		vmcase (LESSTHANEQUAL)
			state[instr->a()].set(!state[instr->c()].lessthan(state, state[instr->b()]));
			vmbreak;
		vmcase (LESSTHAN)
			state[instr->a()].set(state[instr->b()].lessthan(state, state[instr->c()]));
			vmbreak;
		vmcase (EQUALS)
			state[instr->a()].set(state[instr->b()].equals(state, state[instr->c()]));
			vmbreak;
		vmcase (GREATERTHANEQUAL)
			state[instr->a()].set(!state[instr->c()].greaterthan(state, state[instr->b()]));
			vmbreak;
		vmcase (GREATERTHAN)
			state[instr->a()].set(state[instr->b()].greaterthan(state, state[instr->c()]));
			vmbreak;
		vmcase (ADD)
			state[instr->b()].add(state, state[instr->a()], state[instr->c()]);
			vmbreak;
		vmcase (SUBTRACT)
			state[instr->b()].subtract(state, state[instr->a()], state[instr->c()]);
			vmbreak;
		vmcase (MODULO)
			state[instr->b()].modulo(state, state[instr->a()], state[instr->c()]);
			vmbreak;
		vmcase (CONCAT)
			state[instr->b()].set(stringobject::create(state, state[instr->a()].tostring(state) + state[instr->c()].tostring(state)));
			vmbreak;
		vmcase (MULTIPLY)
			state[instr->b()].multiply(state, state[instr->a()], state[instr->c()]);
			vmbreak;
		vmcase (DIVIDE)
			state[instr->b()].divide(state, state[instr->a()], state[instr->c()]);
			vmbreak;
		vmcase (TABLE) {
			auto tbl = tableobject::create(state);
			auto &tmplate = tables[instr->b()];
			for (auto &kv : tmplate.hashpart) {
				object key;
				object value;
				fromtablevalue(key, kv.key);
				fromtablevalue(value, kv.value);
				tbl.rawset(state, key, value);
			}
			int i = 0;
			for (auto &arr : tmplate.arraypart) {
				object val;
				fromtablevalue(val, arr);
				tbl.rawset(state, static_cast<double>(++i), val);
			}
			state[instr->a()].set(tbl);
			vmbreak;
		}
		vmcase (FNEW) {
			auto &fn = *state.memory.allocate<luafunctionobject>(LUAFUNCTION, protos[instr->b()])->get<luafunctionobject>();
			state[instr->a()].set(fn);
			vmbreak;
		}
		default:
			throw exception(string("opcode not implemented: ") + std::to_string(instr->op()));
	}
}

object luafunctionobject::create(softwarestate &state, const prototype &proto) {
	return *state.memory.allocate<luafunctionobject>(LUAFUNCTION, state, proto)->get<luafunctionobject>();
}

luafunctionobject::luafunctionobject(softwarestate &state, const prototype &proto) {
	auto oob = proto.instructions_size();
	size = oob + 1;
	allocated = std::shared_ptr<instruction>(new instruction[size], std::default_delete<instruction[]>());
	allocated.get()[oob] = {
		prototype::OP_RETURN,
		0,
		0,
		0
	};

	auto instructions = allocated.get();

	for (int i = 0; i < proto.instructions_size(); i++) {
		auto &instr = proto.instruction(i);

		if (instr->op() >= prototype::OP_MAX) {
			throw exception(string("unknown opcode: ") + std::to_string(instr->op()));
		}

		instructions[i] = instr;
	}


	for (int i = 0; i < proto.strings_size(); i++) {
		strings.push_back(stringobject::create(state, proto.string(i)));
	}

	for (int i = 0; i < proto.numbers_size(); i++) {
		numbers.push_back(object(proto.number(i)));
	}

	for (int i = 0; i < proto.tables_size(); i++) {
		tables.emplace_back();
		auto &data = tables.back();

		auto &tbl = proto.table(i);
		for (int j = 0; j < tbl.hashpart.size(); j++) {
			auto &hashpart = tbl.hashparts(j);
			data.hashpart.emplace_back(hashpart);
		}
		for (int j = 0; j < tbl.arraypart.size(); j++) {
			auto &arraypart = tbl.arrayparts(j);
			data.arraypart.emplace_back(arraypart);
		}
	}

	for (int i = 0; i < proto.protos_size(); i++) {
		protos.push_back(luafunctionobject(state, proto.proto(i)));
	}

	stacksize = proto.stacksize;
}

luafunctionobject::luafunctionobject() { }

object functionobject::metatable(softwarestate &state) const {
	return state.function_metatable;
}

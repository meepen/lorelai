#include "../object.hpp"
#include "../software.hpp"
#include <exception>
#include <string>
#include <memory>
#include <iostream>

using namespace lorelai;
using namespace lorelai::vm;
#define vmcase(x) case bytecode::instruction_opcode_##x:
#define vmbreak goto instructionstart

struct luafunctionobject::instruction {
	std::uint32_t opcode;
	std::uint32_t a;
	std::uint32_t b;
	std::uint32_t c;
};

void luafunctionobject::fromtablevalue(object &out, const bytecode::tablevalue &data) {
	switch (data.type()) {
	case bytecode::tablevalue_valuetype_CONSTANT:
		switch (data.index()) {
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

	case bytecode::tablevalue_valuetype_NUMBER:
		out.set(numbers[data.index()]);
		break;

	case bytecode::tablevalue_valuetype_STRING:
		out.set(strings[data.index()]);
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
	auto ends = starts + size;

	instruction *instr = starts;
	instruction *next = instr;

	instructionstart:
	instr = next++;
	switch (instr->opcode) {
		vmcase(RETURN)
			return retdata;
		vmcase (JMPIFTRUE)
			if (state[instr->a].tobool(state)) {
				next = starts + instr->b;
			}
			vmbreak;
		vmcase (JMPIFFALSE)
			if (!state[instr->a].tobool(state)) {
				next = starts + instr->b;
			}
			vmbreak;
		vmcase (FORCHECK) {
			auto a = instr->a;
			if (!(state[a + 2].tonumber(state) > 0 ? state[a].tonumber(state) <= state[a + 1].tonumber(state)
				: state[a + 2].tonumber(state) <= 0 && state[a].tonumber(state) >= state[a + 1].tonumber(state))) {
				next = starts + instr->b;
			}
			vmbreak;
		}
		vmcase (JMP)
			next = starts + instr->b;
			vmbreak;
		vmcase (CONSTANT) {
			switch (instr->b) {
			case 0:
				state[instr->a].set(true);
				vmbreak;
			case 1:
				state[instr->a].set(false);
				vmbreak;
			default:
				state[instr->a].unset();
				vmbreak;
			}
			vmbreak;
		}
		vmcase (MOV) {
			auto a = instr->a, b = instr->b, c = instr->c + 1;

			while (c--) {
				state[a++].set(state[b++]);
			}
			vmbreak;
		}
		/*vmcase (CALLM) {
			// A .. A+C-2 = A(A+1 .. A + B, ...)
			for (int i = multres - 1; i >= 0; i--) {
				state->stacktop[i + instr->b + 1].set(state->stacktop[i]);
			}
			for (unsigned i = 0; i <= instr->b; i++) {
				state->stacktop[i].set(state[instr->a + i]);
			}

			state[0].call(state, instr->b + multres, instr->c - 1);
			if (instr->c >= 1) {
				// x res
			}
			else {
				// multres
			}
			vmbreak;
		}*/
		vmcase (CALL) {
			// A .. A+C-1, ... = A(A+1 .. A + B)
			for (int i = 0; i <= instr->b; i++) {
				state->stacktop[i].set(state[instr->a + i]);
			}
			auto old = state->pushstack(instr->b + 1);
			auto retdata = state[0].call(state, instr->b);
			auto nrets = state->popstack(old, retdata);

			for (int i = 0; i < instr->c; i++) {
				state[instr->a + i].set(state->stacktop[i]);
			}

			multres = nrets - instr->c;
			for (int i = instr->c; i < nrets; i++) {
				state->stacktop[i].set(state->stacktop[i + instr->c]);
			}

			vmbreak;
		}
		vmcase (MINUS)
			state[instr->a].set(-state[instr->b].tonumber(state));
			vmbreak;
		vmcase (ENVIRONMENTGET)
			state.registry.index(state, state[instr->a], strings[instr->b]);
			vmbreak;
		vmcase (STRING)
			state[instr->a].set(strings[instr->b]);
			vmbreak;
		vmcase (NUMBER)
			state[instr->a].set(numbers[instr->b]);
			vmbreak;
		vmcase (INDEX)
			state[instr->b].index(state, state[instr->a], state[instr->c]);
			vmbreak;
		vmcase (SETINDEX)
			state[instr->a].setindex(state, state[instr->b], state[instr->c]);
			vmbreak;
		vmcase (NOT)
			state[instr->a].set(!state[instr->b].tobool(state));
			vmbreak;
		vmcase (NOTEQUALS)
			state[instr->a].set(!state[instr->c].equals(state, state[instr->b]));
			vmbreak;
		vmcase (LESSTHANEQUAL)
			state[instr->a].set(!state[instr->c].lessthan(state, state[instr->b]));
			vmbreak;
		vmcase (LESSTHAN)
			state[instr->a].set(state[instr->b].lessthan(state, state[instr->c]));
			vmbreak;
		vmcase (EQUALS)
			state[instr->a].set(state[instr->b].equals(state, state[instr->c]));
			vmbreak;
		vmcase (GREATERTHANEQUAL)
			state[instr->a].set(!state[instr->c].greaterthan(state, state[instr->b]));
			vmbreak;
		vmcase (GREATERTHAN)
			state[instr->a].set(state[instr->b].greaterthan(state, state[instr->c]));
			vmbreak;
		vmcase (ADD)
			state[instr->b].add(state, state[instr->a], state[instr->c]);
			vmbreak;
		vmcase (SUBTRACT)
			state[instr->b].subtract(state, state[instr->a], state[instr->c]);
			vmbreak;
		vmcase (MODULO)
			state[instr->b].modulo(state, state[instr->a], state[instr->c]);
			vmbreak;
		vmcase (CONCAT)
			state[instr->b].set(stringobject::create(state, state[instr->a].tostring(state) + state[instr->c].tostring(state)));
			vmbreak;
		vmcase (MULTIPLY)
			state[instr->b].multiply(state, state[instr->a], state[instr->c]);
			vmbreak;
		vmcase (DIVIDE)
			state[instr->b].divide(state, state[instr->a], state[instr->c]);
			vmbreak;
		vmcase (TABLE) {
			auto tbl = tableobject::create(state);
			auto &tmplate = tables[instr->b];
			for (auto &kv : tmplate.hashpart) {
				object key;
				object value;
				fromtablevalue(key, kv.first);
				fromtablevalue(value, kv.second);
				tbl.rawset(state, key, value);
			}
			int i = 0;
			for (auto &arr : tmplate.arraypart) {
				object val;
				fromtablevalue(val, arr);
				tbl.rawset(state, static_cast<double>(++i), val);
			}
			state[instr->a].set(tbl);
			vmbreak;
		}
		vmcase (FNEW) {
			auto &fn = *state.memory.allocate<luafunctionobject>(LUAFUNCTION, protos[instr->b])->get<luafunctionobject>();
			state[instr->a].set(fn);
			vmbreak;
		}
		default:
			throw exception(string("opcode not implemented: ") + bytecode::instruction_opcode_Name(static_cast<bytecode::instruction_opcode>(instr->opcode)));
	}
}

object luafunctionobject::create(softwarestate &state, const bytecode::prototype &proto) {
	return *state.memory.allocate<luafunctionobject>(LUAFUNCTION, state, proto)->get<luafunctionobject>();
}

luafunctionobject::luafunctionobject(softwarestate &state, const bytecode::prototype &proto) {
	auto oob = proto.instructions_size();
	size = oob + 1;
	allocated = std::shared_ptr<instruction>(new instruction[size], std::default_delete<instruction[]>());
	allocated.get()[oob] = {
		bytecode::instruction_opcode_RETURN,
		0,
		0,
		0
	};

	auto instructions = allocated.get();

	for (int i = 0; i < proto.instructions_size(); i++) {
		auto &instr = proto.instructions(i);

		if (instr.op() > bytecode::instruction_opcode_opcode_MAX) {
			throw exception(string("unknown opcode: ") + bytecode::instruction_opcode_Name(instr.op()));
		}

		instruction &generated = instructions[i];
		generated.opcode = instr.op();
		generated.a = instr.a();
		generated.b = instr.b();
		generated.c = instr.c();
	}

	// add fastlookup alternative jump to necessary ops

	for (int i = 0; i < proto.instructions_size(); i++) {
		auto &patchproto = proto.instructions(i);
		auto &patchinstr = instructions[i];

		switch (patchproto.op()) {
		case bytecode::instruction_opcode_JMP:
		case bytecode::instruction_opcode_JMPIFFALSE:
		case bytecode::instruction_opcode_JMPIFNIL:
		case bytecode::instruction_opcode_JMPIFTRUE:
		case bytecode::instruction_opcode_FORCHECK:
			if (patchinstr.b >= oob) {
				throw exception("invalid bytecode: JMP");
			}
		default:
			break;
		}
	}

	for (int i = 0; i < proto.strings_size(); i++) {
		strings.push_back(stringobject::create(state, proto.strings(i)));
	}

	for (int i = 0; i < proto.numbers_size(); i++) {
		numbers.push_back(object(proto.numbers(i)));
	}

	for (int i = 0; i < proto.tables_size(); i++) {
		auto &tbl = proto.tables(i);
		tabledata data;
		for (int j = 0; j < tbl.hashpart_size(); j++) {
			auto &hashpart = tbl.hashpart(j);
			data.hashpart.push_back(std::make_pair(hashpart.key(), hashpart.value()));
		}
		for (int j = 0; j < tbl.arraypart_size(); j++) {
			data.arraypart.push_back(tbl.arraypart(j));
		}
		tables.push_back(data);
	}

	for (int i = 0; i < proto.protos_size(); i++) {
		protos.push_back(luafunctionobject(state, proto.protos(i)));
	}

	stacksize = proto.stacksize();
}

luafunctionobject::luafunctionobject() { }

object functionobject::metatable(softwarestate &state) const {
	return state.function_metatable;
}

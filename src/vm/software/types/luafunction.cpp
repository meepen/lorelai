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


state::_retdata luafunctionobject::call(softwarestate &state, int nrets, int nargs) {
	int multres = 0;
	state::_retdata retdata {0, 0};

	auto stackptr = &state[0];

	state->top = state->base + stacksize;
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
			if (stackptr[instr->a].tobool(state)) {
				next = starts + instr->b;
			}
			vmbreak;
		vmcase (JMPIFFALSE)
			if (!stackptr[instr->a].tobool(state)) {
				next = starts + instr->b;
			}
			vmbreak;
		vmcase (FORCHECK) {
			auto a = instr->a;
			if (!(stackptr[a + 2].tonumber(state) > 0 ? stackptr[a].tonumber(state) <= stackptr[a + 1].tonumber(state)
				: stackptr[a + 2].tonumber(state) <= 0 && stackptr[a].tonumber(state) >= stackptr[a + 1].tonumber(state))) {
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
				stackptr[instr->a].set(true);
				vmbreak;
			case 1:
				stackptr[instr->a].set(false);
				vmbreak;
			default:
				stackptr[instr->a].unset();
				vmbreak;
			}
			vmbreak;
		}
		vmcase (MOV) {
			auto a = instr->a, b = instr->b, c = instr->c + 1;

			while (c--) {
				stackptr[a++].set(stackptr[b++]);
			}
			vmbreak;
		}
		vmcase (CALLM) {
			// A .. A+C-2 = A(A+1 .. A + B, ...)
			for (int i = multres - 1; i >= 0; i--) {
				stackptr[state->top + i + instr->b + 1].set(stackptr[state->top + i]);
			}
			for (unsigned i = 0; i <= instr->b; i++) {
				stackptr[state->top + i].set(stackptr[instr->a + i]);
			}

			auto old = state->pushpointer(state->base + state->top);
			auto data = stackptr[0];
			auto nret = data.call(state, instr->b + multres, instr->c - 1);
			if (instr->c >= 1) {
				state->poppointer(old, nret, old.base + instr->a, instr->c - 1);
			}
			else {
				multres = state->poppointer(old, nret, old.base + old.top, -1);
			}
			vmbreak;
		}
		vmcase (CALL) {
			// A .. A+C-2 = A(A+1 .. A + B)
			auto old = state->pushpointer(state->base + instr->a);
			auto nret = state[0].call(state, instr->b, instr->c - 1);
			if (instr->c >= 1) {
				state->poppointer(old, nret, old.base + instr->a, instr->c - 1);
			}
			else {
				multres = state->poppointer(old, nret, old.base + old.top, -1);
			}
			vmbreak;
		}
		vmcase (MINUS)
			stackptr[instr->a].set(-stackptr[instr->b].tonumber(state));
			vmbreak;
		vmcase (ENVIRONMENTGET)
			state.registry.index(state, stackptr[instr->a], strings[instr->b]);
			vmbreak;
		vmcase (STRING)
			stackptr[instr->a].set(strings[instr->b]);
			vmbreak;
		vmcase (NUMBER)
			stackptr[instr->a].set(numbers[instr->b]);
			vmbreak;
		vmcase (INDEX)
			stackptr[instr->b].index(state, stackptr[instr->a], stackptr[instr->c]);
			vmbreak;
		vmcase (SETINDEX)
			stackptr[instr->a].setindex(state, stackptr[instr->b], stackptr[instr->c]);
			vmbreak;
		vmcase (NOT)
			stackptr[instr->a].set(!stackptr[instr->b].tobool(state));
			vmbreak;
		vmcase (NOTEQUALS)
			stackptr[instr->a].set(!stackptr[instr->c].equals(state, stackptr[instr->b]));
			vmbreak;
		vmcase (LESSTHANEQUAL)
			stackptr[instr->a].set(!stackptr[instr->c].lessthan(state, stackptr[instr->b]));
			vmbreak;
		vmcase (LESSTHAN)
			stackptr[instr->a].set(stackptr[instr->b].lessthan(state, stackptr[instr->c]));
			vmbreak;
		vmcase (EQUALS)
			stackptr[instr->a].set(stackptr[instr->b].equals(state, stackptr[instr->c]));
			vmbreak;
		vmcase (GREATERTHANEQUAL)
			stackptr[instr->a].set(!stackptr[instr->c].greaterthan(state, stackptr[instr->b]));
			vmbreak;
		vmcase (GREATERTHAN)
			stackptr[instr->a].set(stackptr[instr->b].greaterthan(state, stackptr[instr->c]));
			vmbreak;
		vmcase (ADD)
			stackptr[instr->b].add(state, stackptr[instr->a], stackptr[instr->c]);
			vmbreak;
		vmcase (SUBTRACT)
			stackptr[instr->b].subtract(state, stackptr[instr->a], stackptr[instr->c]);
			vmbreak;
		vmcase (MODULO)
			stackptr[instr->b].modulo(state, stackptr[instr->a], stackptr[instr->c]);
			vmbreak;
		vmcase (CONCAT)
			stackptr[instr->b].concat(state, stackptr[instr->a], stackptr[instr->c]);
			vmbreak;
		vmcase (MULTIPLY)
			stackptr[instr->b].multiply(state, stackptr[instr->a], stackptr[instr->c]);
			vmbreak;
		vmcase (DIVIDE)
			stackptr[instr->b].divide(state, stackptr[instr->a], stackptr[instr->c]);
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
			stackptr[instr->a].set(tbl);
			vmbreak;
		}
		default:
			throw exception(string("opcode not implemented: ") + bytecode::instruction_opcode_Name(static_cast<bytecode::instruction_opcode>(instr->opcode)));
	}
}

object luafunctionobject::create(softwarestate &state, std::shared_ptr<bytecode::prototype> proto) {
	return *state.memory.allocate<luafunctionobject>(LUAFUNCTION, state, proto)->get<luafunctionobject>();
}

luafunctionobject::luafunctionobject(softwarestate &state, std::shared_ptr<bytecode::prototype> proto) {
	auto oob = proto->instructions_size();
	size = oob + 1;
	allocated = std::shared_ptr<instruction>(new instruction[size], std::default_delete<instruction[]>());
	allocated.get()[oob] = {
		bytecode::instruction_opcode_RETURN,
		0,
		0,
		0
	};

	auto instructions = allocated.get();

	for (int i = 0; i < proto->instructions_size(); i++) {
		auto &instr = proto->instructions(i);

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

	for (int i = 0; i < proto->instructions_size(); i++) {
		auto &patchproto = proto->instructions(i);
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

	for (int i = 0; i < proto->strings_size(); i++) {
		strings.push_back(stringobject::create(state, proto->strings(i)));
	}

	for (int i = 0; i < proto->numbers_size(); i++) {
		numbers.push_back(object(proto->numbers(i)));
	}

	for (int i = 0; i < proto->tables_size(); i++) {
		auto &tbl = proto->tables(i);
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

	stacksize = proto->stacksize();
}

luafunctionobject::luafunctionobject() { }

object functionobject::metatable(softwarestate &state) const {
	return state.function_metatable;
}

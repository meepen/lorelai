#include "../object.hpp"
#include "../software.hpp"
#include "../container.hpp"
#include <exception>
#include <string>
#include <memory>

using namespace lorelai;
using namespace lorelai::vm;
#define vmcase(x) case bytecode::instruction_opcode_##x:

struct luafunctionobject::instruction {
	std::uint32_t opcode;
	std::uint32_t a;
	std::uint32_t b;
	std::uint32_t c;
};


state::_retdata luafunctionobject::call(softwarestate &state, int nrets, int nargs) {
	int multres = 0;
	state::_retdata retdata {0, 0};

	auto stackptr = &state[0];

	state->top = state->base + stacksize;
	auto starts = allocated.get();
	auto ends = starts + size;

	instruction *instr = starts;

	while (true) {
		instruction *next = instr + 1;
		switch (instr->opcode) {
		vmcase(RETURN) {
			return retdata;
		}
		vmcase (JMPIFTRUE) {
			if (stackptr[instr->a].tobool(state)) {
				next = starts + instr->b;
			}
			break;
		}
		vmcase (JMPIFFALSE) {
			if (!stackptr[instr->a].tobool(state)) {
				next = starts + instr->b;
			}
			break;
		}
		vmcase (JMP) {
			next = starts + instr->b;
			break;
		}
		vmcase (CONSTANT) {
			switch (instr->b) {
			case 0:
				stackptr[instr->a].set(true);
				break;
			case 1:
				stackptr[instr->a].set(false);
				break;
			default:
				stackptr[instr->a].unset();
				break;
			}
			break;
		}
		vmcase (MINUS) {
			stackptr[instr->a].set(-stackptr[instr->b].tonumber(state));
			break;
		}
		vmcase (MOV) {
			auto a = instr->a, b = instr->b, c = instr->c;

			while (c--) {
				stackptr[a++].set(stackptr[b++]);
			}
			break;
		}
		vmcase (CALLM) {
			// A .. A+C-2 = A(A+1 .. A + B, ...)
			for (int i = multres - 1; i >= 0; i--) {
				stackptr[state->top + i + instr->b + 1].set(stackptr[state->top + i]);
			}
			for (int i = 0; i <= instr->b; i++) {
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
			break;
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
			break;
		}
		vmcase (ENVIRONMENTGET) {
			object &index = strings[instr->b];

			state.registry.index(state, stackptr[instr->a], index);
			break;
		}
		vmcase (STRING) {
			stackptr[instr->a].set(strings[instr->b]);
			break;
		}
		vmcase (NUMBER) {
			stackptr[instr->a].set(numbers[instr->b]);
			break;
		}
		vmcase (INDEX) {
			auto ref = stackptr[instr->b];
			ref.index(state, stackptr[instr->a], stackptr[instr->c]);
			break;
		}
		vmcase (NOT) {
			stackptr[instr->a].set(!stackptr[instr->b].tobool(state));
			break;
		}
		vmcase (NOTEQUALS) {
			stackptr[instr->a].set(!stackptr[instr->c].equals(state, stackptr[instr->b]));
			break;
		}
		vmcase (LESSTHANEQUAL) {
			stackptr[instr->a].set(!stackptr[instr->c].lessthan(state, stackptr[instr->b]));
			break;
		}
		vmcase (LESSTHAN) {
			stackptr[instr->a].set(stackptr[instr->b].lessthan(state, stackptr[instr->c]));
			break;
		}
		vmcase (EQUALS) {
			stackptr[instr->a].set(stackptr[instr->b].equals(state, stackptr[instr->c]));
			break;
		}
		vmcase (GREATERTHANEQUAL) {
			stackptr[instr->a].set(!stackptr[instr->c].greaterthan(state, stackptr[instr->b]));
			break;
		}
		vmcase (GREATERTHAN) {
			stackptr[instr->a].set(stackptr[instr->b].greaterthan(state, stackptr[instr->c]));
			break;
		}
		vmcase (ADD) {
			stackptr[instr->b].add(state, stackptr[instr->a], stackptr[instr->c]);
			break;
		}
		vmcase (SUBTRACT) {
			stackptr[instr->b].subtract(state, stackptr[instr->a], stackptr[instr->c]);
			break;
		}
		vmcase (MODULO) {
			stackptr[instr->b].modulo(state, stackptr[instr->a], stackptr[instr->c]);
			break;
		}
		vmcase (CONCAT) {
			stackptr[instr->b].concat(state, stackptr[instr->a], stackptr[instr->c]);
			break;
		}
		vmcase (MULTIPLY) {
			stackptr[instr->b].multiply(state, stackptr[instr->a], stackptr[instr->c]);
			break;
		}
		vmcase (DIVIDE) {
			stackptr[instr->b].divide(state, stackptr[instr->a], stackptr[instr->c]);
			break;
		}
		default:
			throw exception("opcode not implemented: " + bytecode::instruction_opcode_Name(instr->opcode));
		}

		instr = next;
	}

	return retdata;
}

object luafunctionobject::create(softwarestate &state, std::shared_ptr<bytecode::prototype> proto) {
	return *state.memory.allocate<luafunctionobject>(LUAFUNCTION, state, proto)->get<luafunctionobject>();
}

luafunctionobject::luafunctionobject(softwarestate &state, std::shared_ptr<bytecode::prototype> proto) {
	auto oob = proto->instructions_size();
	size = oob;
	allocated = std::shared_ptr<instruction>(new instruction[oob], std::default_delete<instruction[]>());

	auto instructions = allocated.get();

	for (int i = 0; i < proto->instructions_size(); i++) {
		auto &instr = proto->instructions(i);

		if (instr.op() >= bytecode::instruction_opcode_opcode_MAX) {
			throw exception("unknown opcode: " + instr.op());
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

	stacksize = proto->stacksize();
}

luafunctionobject::luafunctionobject() { }

object functionobject::metatable(softwarestate &state) const {
	return state.function_metatable;
}

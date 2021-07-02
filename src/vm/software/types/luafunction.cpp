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

struct _running {
	softwarestate &state;
	luafunctionobject *obj;
	int nrets;
	int nargs;
	int multres = 0;
	state::_retdata retdata {0, 0};
};

state::_retdata luafunctionobject::call(softwarestate &state, int nrets, int nargs) {
	_running run { state, this, nrets, nargs };

	state->top = state->base + stacksize;
	auto starts = allocated.get();
	auto ends = starts + size;

	instruction *instr = starts;

	while (instr < ends && instr >= starts) {
		instruction *next = instr + 1;
		switch (instr->opcode) {
		vmcase (JMPIFTRUE) {
			if (run.state[instr->a].tobool(run.state)) {
				next = starts + instr->b;
			}
			break;
		}
		vmcase (JMPIFFALSE) {
			if (!run.state[instr->a].tobool(run.state)) {
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
				run.state[instr->a].set(true);
				break;
			case 1:
				run.state[instr->a].set(false);
				break;
			default:
				run.state[instr->a].unset();
				break;
			}
			break;
		}
		vmcase (MINUS) {
			run.state[instr->a].set(-run.state[instr->b].tonumber(run.state));
			break;
		}
		vmcase (MOV) {
			auto a = instr->a, b = instr->b, c = instr->c;

			while (c--) {
				run.state[a++].set(run.state[b++]);
			}
			break;
		}
		vmcase (CALLM) {
			// A .. A+C-2 = A(A+1 .. A + B, ...)
			for (int i = run.multres - 1; i >= 0; i--) {
				run.state[run.state->top + i + instr->b + 1].set(run.state[run.state->top + i]);
			}
			for (int i = 0; i <= instr->b; i++) {
				run.state[run.state->top + i].set(run.state[instr->a + i]);
			}

			auto old = run.state->pushpointer(run.state->base + run.state->top);
			auto data = run.state[0];
			auto nret = data.call(run.state, instr->b + run.multres, instr->c - 1);
			if (instr->c >= 1) {
				run.state->poppointer(old, nret, old.base + instr->a, instr->c - 1);
			}
			else {
				run.multres = run.state->poppointer(old, nret, old.base + old.top, -1);
			}
			break;
		}
		vmcase (CALL) {
			// A .. A+C-2 = A(A+1 .. A + B)
			auto old = run.state->pushpointer(run.state->base + instr->a);
			auto data = run.state[0];
			auto nret = data.call(run.state, instr->b, instr->c - 1);
			if (instr->c >= 1) {
				run.state->poppointer(old, nret, old.base + instr->a, instr->c - 1);
			}
			else {
				run.multres = run.state->poppointer(old, nret, old.base + old.top, -1);
			}
			break;
		}
		vmcase (ENVIRONMENTGET) {
			object &index = run.obj->strings[instr->b];

			run.state.registry.index(run.state, run.state[instr->a], index);
			break;
		}
		vmcase (STRING) {
			run.state[instr->a] = run.obj->strings[instr->b];
			break;
		}
		vmcase (NUMBER) {
			run.state[instr->a] = run.obj->numbers[instr->b];
			break;
		}
		vmcase (INDEX) {
			auto ref = run.state[instr->b];
			ref.index(run.state, run.state[instr->a], run.state[instr->c]);
			break;
		}
		vmcase (NOT) {
			run.state[instr->a].set(!run.state[instr->b].tobool(run.state));
			break;
		}
		vmcase (NOTEQUALS) {
			run.state[instr->a].set(!run.state[instr->c].equals(run.state, run.state[instr->b]));
			break;
		}
		vmcase (LESSTHANEQUAL) {
			run.state[instr->a].set(!run.state[instr->c].lessthan(run.state, run.state[instr->b]));
			break;
		}
		vmcase (LESSTHAN) {
			run.state[instr->a].set(run.state[instr->b].lessthan(run.state, run.state[instr->c]));
			break;
		}
		vmcase (EQUALS) {
			run.state[instr->a].set(run.state[instr->b].equals(run.state, run.state[instr->c]));
			break;
		}
		vmcase (GREATERTHANEQUAL) {
			run.state[instr->a].set(!run.state[instr->c].greaterthan(run.state, run.state[instr->b]));
			break;
		}
		vmcase (GREATERTHAN) {
			run.state[instr->a].set(run.state[instr->b].greaterthan(run.state, run.state[instr->c]));
			break;
		}
		vmcase (ADD) {
			run.state[instr->b].add(run.state, run.state[instr->a], run.state[instr->c]);
			break;
		}
		vmcase (SUBTRACT) {
			run.state[instr->b].subtract(run.state, run.state[instr->a], run.state[instr->c]);
			break;
		}
		vmcase (MODULO) {
			run.state[instr->b].modulo(run.state, run.state[instr->a], run.state[instr->c]);
			break;
		}
		vmcase (CONCAT) {
			run.state[instr->b].concat(run.state, run.state[instr->a], run.state[instr->c]);
			break;
		}
		vmcase (MULTIPLY) {
			run.state[instr->b].multiply(run.state, run.state[instr->a], run.state[instr->c]);
			break;
		}
		vmcase (DIVIDE) {
			run.state[instr->b].divide(run.state, run.state[instr->a], run.state[instr->c]);
			break;
		}
		default:
			throw exception("opcode not implemented: " + bytecode::instruction_opcode_Name(instr->opcode));
		}

		instr = next;
	}

	return run.retdata;
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

#include "../object.hpp"
#include "../software.hpp"
#include <exception>
#include <string>

using namespace lorelai;
using namespace lorelai::vm;

std::shared_ptr<object> lorelai::vm::function_metatable = nullptr;

using func = state::_retdata *(*)(softwarestate &state, size_t &nextinstruction, int nrets, int nargs, const bytecode::instruction &instr, const std::shared_ptr<bytecode::prototype> proto, state::_retdata &retdata);

#define OPCODE_FUNCTION(t) static state::_retdata *t(softwarestate &state, size_t &nextinstruction, int nrets, int nargs, const bytecode::instruction &instr, const std::shared_ptr<bytecode::prototype> proto, state::_retdata &retdata)

OPCODE_FUNCTION(openvironmentget) {
	objectcontainer index = std::make_shared<stringobject>(proto->strings(instr.b()));

	state.registry->rawget(state, state[instr.a()], index);

	return nullptr;
}

OPCODE_FUNCTION(opnumber) {
	state[instr.a()] = std::make_shared<numberobject>(proto->numbers(instr.b()));

	return nullptr;
}

OPCODE_FUNCTION(opcall) {
	 // A .. A+C-2 = A(A+1 .. A + B)
	auto old = state->pushpointer(state->base + instr.a());
	auto data = state[0];
	auto nret = data->call(state, instr.c() - 1, instr.b());
	if (instr.c() >= 1) {
		state->poppointer(old, nret, state->base + instr.a(), instr.c() - 1);
	}
	else {
		state->poppointer(old, nret, state->base + state->top, -1);
	}

	return nullptr;
}

OPCODE_FUNCTION(opmov) {
	// A .. A+C = B .. B+C

	int a = instr.a(), b = instr.b();

	for (int i = 0; i < instr.c(); i++) {
		state[a + i] = state[b + i];
	}
	
}

static std::map<bytecode::instruction_opcode, func> opcode_map = {
	{ bytecode::instruction_opcode_ENVIRONMENTGET, openvironmentget },
	{ bytecode::instruction_opcode_NUMBER, opnumber },
	{ bytecode::instruction_opcode_CALL, opcall },
	{ bytecode::instruction_opcode_MOV, opmov },
};

class exception : public std::exception {
public:
	exception(std::string str) : err(str) { }

	const char *what() const noexcept {
		return err.c_str();
	}

public:
	std::string err;
};

state::_retdata luafunctionobject::call(softwarestate &state, int nrets, int nargs) {
	auto oldtop = state->top;
	state::_retdata retdata { 0, 0 };

	size_t nextinstruction = 0;
	auto max = data->instructions_size();
	
	while (nextinstruction < max) {
		auto &instr = data->instructions(nextinstruction++);
		auto found = opcode_map.find(instr.op());
		if (found == opcode_map.end()) {
			throw exception("Unknown opcode " + bytecode::instruction_opcode_Name(instr.op()));
		}
		if (found->second(state, nextinstruction, nrets, nargs, instr, data, retdata)) {
			break;
		}
	}

	return retdata;
}
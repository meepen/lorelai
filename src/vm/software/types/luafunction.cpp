#include "../object.hpp"
#include "../software.hpp"
#include <exception>
#include <string>

using namespace lorelai;
using namespace lorelai::vm;

std::shared_ptr<object> lorelai::vm::function_metatable = nullptr;

using func = bool (*)(softwarestate &state, size_t &nextinstruction, std::shared_ptr<object> *stack, size_t nrets, size_t nargs, const bytecode::instruction &instr, const std::shared_ptr<bytecode::prototype> proto);

#define OPCODE_FUNCTION(t) bool t(softwarestate &state, size_t &nextinstruction, std::shared_ptr<object> *stack, size_t nrets, size_t nargs, const bytecode::instruction &instr, const std::shared_ptr<bytecode::prototype> proto)

OPCODE_FUNCTION(openvironmentget) {
	objectcontainer index = std::make_shared<stringobject>(proto->strings(instr.b()));

	state.registry->rawget(state, stack[instr.a()], index);

	return false;
}

OPCODE_FUNCTION(opnumber) {
	stack[instr.a()] = std::make_shared<numberobject>(proto->numbers(instr.b()));

	return false;
}

OPCODE_FUNCTION(opcall) {
	 // A .. A+C-2 = A(A+1 .. A + B)
	auto data = stack[instr.a()];
	auto nret = data->call(state, &stack[instr.a()], instr.c() - 1, instr.b());

	for (auto i = nret; i < instr.b(); i++) {
		stack[i] = std::make_shared<nilobject>();
	}

	return false;
}

static std::map<bytecode::instruction_opcode, func> opcode_map = {
	{ bytecode::instruction_opcode_ENVIRONMENTGET, openvironmentget },
	{ bytecode::instruction_opcode_NUMBER, opnumber },
	{ bytecode::instruction_opcode_CALL, opcall },
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

size_t luafunctionobject::call(softwarestate &state, objectcontainer *out, size_t nrets, size_t nargs) {
	size_t nextinstruction = 0;
	auto max = data->instructions_size();
	
	while (nextinstruction < max) {
		auto &instr = data->instructions(nextinstruction++);
		auto found = opcode_map.find(instr.op());
		if (found == opcode_map.end()) {
			throw exception("Unknown opcode " + bytecode::instruction_opcode_Name(instr.op()));
		}
		if (found->second(state, nextinstruction, out, nrets, nargs, instr, data)) {
			break;
		}
	}
}
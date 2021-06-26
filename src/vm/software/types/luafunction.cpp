#include "../object.hpp"
#include "../software.hpp"
#include <exception>
#include <string>

using namespace lorelai;
using namespace lorelai::vm;

std::shared_ptr<object> lorelai::vm::function_metatable = nullptr;

struct _running {
	softwarestate &state;
	int nrets;
	int nargs;
	std::shared_ptr<bytecode::prototype> proto;
	state::_retdata retdata {0, 0};
	int multres = 0;
	size_t nextinstruction = 0;
};
using func = state::_retdata *(*)(_running &run, const bytecode::instruction &instr);

#define OPCODE_FUNCTION(t) static state::_retdata *t(_running &run, const bytecode::instruction &instr)

#define MATHOPS(fn) \
	fn(MODULO, modulo) \
	fn(SUBTRACT, subtract) \
	fn(ADD, add) \
	fn(DIVIDE, divide) \
	fn(MULTIPLY, multiply) \
	fn(CONCAT, concat) \
	fn(POWER, power)
#define OPMAPFUNC(opcode, name, arg...) { bytecode::instruction_opcode_##opcode, op##name },

// A = B - C
#define MATHFUNC(opcode, name) \
OPCODE_FUNCTION(op##name) { \
	run.state[instr.b()]->name(run.state, run.state[instr.a()], run.state[instr.b()], run.state[instr.c()]); \
 \
	return nullptr; \
}

MATHOPS(MATHFUNC)

#define COMPAREOPS(fn) \
	fn(LESSTHAN, lessthan) \
	fn(LESSTHANEQUAL, lessthanequal) \
	fn(GREATERTHAN, greaterthan) \
	fn(GREATERTHANEQUAL, greaterthanequal) \
	fn(EQUALS, equals) \
	fn(NOTEQUALS, notequals)

#define LOGICOPS(fn) \
	fn(and, AND, and, &&) \
	fn(or, OR, or, ||)

OPCODE_FUNCTION(opindex) {
	// A = B [ C ]

	auto ref = run.state[instr.b()];
	ref->index(run.state, run.state[instr.a()], run.state[instr.c()]);

	return nullptr;
}

OPCODE_FUNCTION(openvironmentget) {
	objectcontainer index = std::make_shared<stringobject>(run.proto->strings(instr.b()));

	run.state.registry->rawget(run.state, run.state[instr.a()], index);

	return nullptr;
}

OPCODE_FUNCTION(opnumber) {
	run.state[instr.a()] = std::make_shared<numberobject>(run.proto->numbers(instr.b()));

	return nullptr;
}

OPCODE_FUNCTION(opstring) {
	run.state[instr.a()] = std::make_shared<stringobject>(run.proto->strings(instr.b()));

	return nullptr;
}

OPCODE_FUNCTION(opcall) {
	// A .. A+C-2 = A(A+1 .. A + B)
	auto old = run.state->pushpointer(run.state->base + instr.a());
	auto data = run.state[0];
	auto nret = data->call(run.state, instr.c() - 1, instr.b());
	if (instr.c() >= 1) {
		run.state->poppointer(old, nret, old.base + instr.a(), instr.c() - 1);
	}
	else {
		run.multres = run.state->poppointer(old, nret, old.base + old.top, -1);
	}

	return nullptr;
}

OPCODE_FUNCTION(opcallm) {
	// A .. A+C-2 = A(A+1 .. A + B, ...)
	for (int i = run.multres - 1; i >= 0; i--) {
		run.state[run.state->top + i + instr.b() + 1] = run.state[run.state->top + i];
	}
	for (int i = 0; i <= instr.b(); i++) {
		run.state[run.state->top + i] = run.state[instr.a() + i];
	}

	auto old = run.state->pushpointer(run.state->base + run.state->top);
	auto data = run.state[0];
	auto nret = data->call(run.state, instr.c() - 1, instr.b() + run.multres);
	if (instr.c() >= 1) {
		run.state->poppointer(old, nret, old.base + instr.a(), instr.c() - 1);
	}
	else {
		run.multres = run.state->poppointer(old, nret, old.base + old.top, -1);
	}

	return nullptr;
}

OPCODE_FUNCTION(opmov) {
	// A .. A+C = B .. B+C

	int a = instr.a(), b = instr.b();

	for (int i = 0; i < instr.c(); i++) {
		run.state[a + i] = run.state[b + i];
	}
	
}

static std::map<bytecode::instruction_opcode, func> opcode_map = {
	{ bytecode::instruction_opcode_ENVIRONMENTGET, openvironmentget },
	{ bytecode::instruction_opcode_NUMBER, opnumber },
	{ bytecode::instruction_opcode_CALL, opcall },
	{ bytecode::instruction_opcode_CALLM, opcallm },
	{ bytecode::instruction_opcode_MOV, opmov },
	{ bytecode::instruction_opcode_STRING, opstring },
	{ bytecode::instruction_opcode_INDEX, opindex },
	MATHOPS(OPMAPFUNC)
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
	_running run { state, nrets, nargs, data };

	state->top = state->base + data->stacksize();

	size_t nextinstruction = 0;
	auto max = data->instructions_size();
	
	while (nextinstruction < max) {
		auto &instr = data->instructions(nextinstruction++);
		auto found = opcode_map.find(instr.op());
		if (found == opcode_map.end()) {
			throw exception("Unknown opcode " + bytecode::instruction_opcode_Name(instr.op()));
		}
		if (found->second(run, instr)) {
			break;
		}
	}

	return run.retdata;
}
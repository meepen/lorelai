#include "../object.hpp"
#include "../software.hpp"
#include <exception>
#include <string>

using namespace lorelai;
using namespace lorelai::vm;

objectcontainer lorelai::vm::function_metatable = nullptr;

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
	run.state[instr.b()]-> name (run.state, run.state[instr.a()], run.state[instr.c()]); \
 \
	return nullptr; \
}

MATHOPS(MATHFUNC)

#define COMPAREOPS(fn) \
	fn(LESSTHAN, lessthan) \
	fn(GREATERTHAN, greaterthan) \
	fn(EQUALS, equals)

#define OPFUNC(opcode, name) \
OPCODE_FUNCTION(op##name) { \
	run.state[instr.a()] = boolobject::create(run.state, run.state[instr.b()]-> name (run.state, run.state[instr.c()])); \
 \
	return nullptr; \
}


OPCODE_FUNCTION(opgreaterthanequal) {
	run.state[instr.a()] = boolobject::create(run.state, !run.state[instr.c()]->greaterthan(run.state, run.state[instr.b()]));

	return nullptr;
}

OPCODE_FUNCTION(oplessthanequal) {
	run.state[instr.a()] = boolobject::create(run.state, !run.state[instr.c()]->lessthan(run.state, run.state[instr.b()]));

	return nullptr;
}

OPCODE_FUNCTION(opnotequals) {
	run.state[instr.a()] = boolobject::create(run.state, !run.state[instr.c()]->equals(run.state, run.state[instr.b()]));

	return nullptr;
}

OPCODE_FUNCTION(opnot) {
	run.state[instr.a()] = boolobject::create(run.state, !run.state[instr.b()]->tobool(run.state));

	return nullptr;
}

OPCODE_FUNCTION(opminus) {
	run.state[instr.a()] = numberobject::create(run.state, -run.state[instr.b()]->tonumber(run.state));

	return nullptr;
}

COMPAREOPS(OPFUNC)

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
	objectcontainer index = stringobject::create(run.state, run.proto->strings(instr.b()));\

	run.state.registry->index(run.state, run.state[instr.a()], index);\

	return nullptr;
}

OPCODE_FUNCTION(opnumber) {
	run.state[instr.a()] = numberobject::create(run.state, run.proto->numbers(instr.b()));

	return nullptr;
}

OPCODE_FUNCTION(opstring) {
	run.state[instr.a()] = stringobject::create(run.state, run.proto->strings(instr.b()));

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

	int a = instr.a(), b = instr.b(), c = instr.c();

	while (c--) {
		run.state[a++] = run.state[b++];
	}
	
	return nullptr;
}

// ({true, false, nil})[B]
OPCODE_FUNCTION(opconstant) {
	switch (instr.b()) {
	case 0:
		run.state[instr.a()] = boolobject::create(run.state, true);
		break;
	case 1:
		run.state[instr.a()] = boolobject::create(run.state, false);
		break;
	default:
		run.state[instr.a()] = nilobject::create(run.state);
		break;
	}

	return nullptr;
}

/*
		JMP           = 34; // JMP(instruction(B))
		JMPIFTRUE     = 35; // if (A) { JMP(instruction(B)) }
		JMPIFFALSE    = 36; // if (!A) { JMP(instruction(B)) }
		JMPIFNIL      = 37; // if (A == nil) { JMP(instruction(B)) }
*/

OPCODE_FUNCTION(opjmp) {
	run.nextinstruction = instr.b();

	return nullptr;
}

OPCODE_FUNCTION(opjmpiffalse) {
	if (!run.state[instr.a()]->tobool(run.state)) {
		run.nextinstruction = instr.b();
	}

	return nullptr;
}

OPCODE_FUNCTION(opjmpiftrue) {
	if (run.state[instr.a()]->tobool(run.state)) {
		run.nextinstruction = instr.b();
	}

	return nullptr;
}

static std::map<bytecode::instruction_opcode, func> opcode_map = {
	{ bytecode::instruction_opcode_ENVIRONMENTGET, openvironmentget },
	{ bytecode::instruction_opcode_NUMBER, opnumber },
	{ bytecode::instruction_opcode_CALL, opcall },
	{ bytecode::instruction_opcode_CALLM, opcallm },
	{ bytecode::instruction_opcode_MOV, opmov },
	{ bytecode::instruction_opcode_STRING, opstring },
	{ bytecode::instruction_opcode_INDEX, opindex },
	{ bytecode::instruction_opcode_GREATERTHANEQUAL, opgreaterthanequal },
	{ bytecode::instruction_opcode_LESSTHANEQUAL, oplessthanequal },
	{ bytecode::instruction_opcode_NOTEQUALS, opnotequals },
	{ bytecode::instruction_opcode_NOT, opnot },
	{ bytecode::instruction_opcode_MINUS, opminus },
	{ bytecode::instruction_opcode_CONSTANT, opconstant },
	{ bytecode::instruction_opcode_JMP, opjmp },
	{ bytecode::instruction_opcode_JMPIFFALSE, opjmpiffalse },
	{ bytecode::instruction_opcode_JMPIFTRUE, opjmpiftrue },
	MATHOPS(OPMAPFUNC)
	COMPAREOPS(OPMAPFUNC)
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

	auto max = data->instructions_size();

	while (run.nextinstruction < max) {
		auto &instr = data->instructions(run.nextinstruction++);
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

objectcontainer luafunctionobject::create(softwarestate &state, std::shared_ptr<bytecode::prototype> proto) {
	return state.luafunctionallocator.take(proto);
}
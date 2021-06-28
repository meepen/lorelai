#include "../object.hpp"
#include "../software.hpp"
#include <exception>
#include <string>
#include <memory>

using namespace lorelai;
using namespace lorelai::vm;

using luaopcode = luafunctionobject::instruction *const (*)(struct _running &run, const luafunctionobject::instruction &instr);
luaopcode luaopcodes[bytecode::instruction_opcode_opcode_MAX] = { nullptr };

struct luafunctionobject::instruction {
	luaopcode opcode;
	std::uint32_t a;
	std::uint32_t b;
	std::uint32_t c;

	luafunctionobject::instruction *fastlookup[2] = { nullptr, nullptr };
};

struct _running {
	softwarestate &state;
	luafunctionobject *obj;
	int nrets;
	int nargs;
	int multres = 0;
	state::_retdata retdata {0, 0};
};

#define OPCODE_FUNCTION(t) static luafunctionobject::instruction *const t(_running &run, const luafunctionobject::instruction &instr)

#define MATHOPS(fn) \
	fn(MODULO, modulo) \
	fn(SUBTRACT, subtract) \
	fn(ADD, add) \
	fn(DIVIDE, divide) \
	fn(MULTIPLY, multiply) \
	fn(CONCAT, concat) \
	fn(POWER, power)

#define OPMAPFUNC(opcode, name, arg...) luaopcodes[bytecode::instruction_opcode_##opcode] = op##name;

// A = B - C
#define MATHFUNC(opcode, name) \
OPCODE_FUNCTION(op##name) { \
	run.state[instr.b] . name (run.state, run.state[instr.a], run.state[instr.c]); \
 \
	return instr.fastlookup[0]; \
}

MATHOPS(MATHFUNC)

#define COMPAREOPS(fn) \
	fn(LESSTHAN, lessthan) \
	fn(GREATERTHAN, greaterthan) \
	fn(EQUALS, equals)

#define OPFUNC(opcode, name) \
OPCODE_FUNCTION(op##name) { \
	run.state[instr.a].set(run.state[instr.b] . name (run.state, run.state[instr.c])); \
 \
	return instr.fastlookup[0]; \
}

OPCODE_FUNCTION(oplessthan) {
	run.state[instr.a].set(run.state[instr.b].lessthan(run.state, run.state[instr.c]));

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opgreaterthan) {
	run.state[instr.a].set(run.state[instr.b].greaterthan(run.state, run.state[instr.c]));

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opequals) {
	run.state[instr.a].set(run.state[instr.b].equals(run.state, run.state[instr.c]));

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opgreaterthanequal) {
	run.state[instr.a].set(!run.state[instr.c].greaterthan(run.state, run.state[instr.b]));

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(oplessthanequal) {
	run.state[instr.a].set(!run.state[instr.c].lessthan(run.state, run.state[instr.b]));

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opnotequals) {
	run.state[instr.a].set(!run.state[instr.c].equals(run.state, run.state[instr.b]));

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opnot) {
	run.state[instr.a].set(!run.state[instr.b].tobool(run.state));

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opminus) {
	run.state[instr.a].set(-run.state[instr.b].tonumber(run.state));

	return instr.fastlookup[0];
}

#define LOGICOPS(fn) \
	fn(and, AND, and, &&) \
	fn(or, OR, or, ||)

OPCODE_FUNCTION(opindex) {
	// A = B [ C ]

	auto ref = run.state[instr.b];
	ref.index(run.state, run.state[instr.a], run.state[instr.c]);

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(openvironmentget) {
	object &index = run.obj->strings[instr.b];

	run.state.registry.index(run.state, run.state[instr.a], index);

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opnumber) {
	run.state[instr.a] = run.obj->numbers[instr.b];

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opstring) {
	run.state[instr.a] = run.obj->strings[instr.b];

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opcall) {
	// A .. A+C-2 = A(A+1 .. A + B)
	auto old = run.state->pushpointer(run.state->base + instr.a);
	auto data = run.state[0];
	auto nret = data.call(run.state, instr.b, instr.c - 1);
	if (instr.c >= 1) {
		run.state->poppointer(old, nret, old.base + instr.a, instr.c - 1);
	}
	else {
		run.multres = run.state->poppointer(old, nret, old.base + old.top, -1);
	}

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opcallm) {
	// A .. A+C-2 = A(A+1 .. A + B, ...)
	for (int i = run.multres - 1; i >= 0; i--) {
		run.state[run.state->top + i + instr.b + 1] = run.state[run.state->top + i];
	}
	for (int i = 0; i <= instr.b; i++) {
		run.state[run.state->top + i] = run.state[instr.a + i];
	}

	auto old = run.state->pushpointer(run.state->base + run.state->top);
	auto data = run.state[0];
	auto nret = data.call(run.state, instr.b + run.multres, instr.c - 1);
	if (instr.c >= 1) {
		run.state->poppointer(old, nret, old.base + instr.a, instr.c - 1);
	}
	else {
		run.multres = run.state->poppointer(old, nret, old.base + old.top, -1);
	}

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opmov) {
	// A .. A+C = B .. B+C

	auto a = instr.a, b = instr.b, c = instr.c;

	while (c--) {
		run.state[a++].set(run.state[b++]);
	}
	
	return instr.fastlookup[0];
}

// ({true, false, nil})[B]
OPCODE_FUNCTION(opconstant) {
	switch (instr.b) {
	case 0:
		run.state[instr.a].set(true);
		break;
	case 1:
		run.state[instr.a].set(false);
		break;
	default:
		run.state[instr.a].unset();
		break;
	}

	return instr.fastlookup[0];
}

/*
		JMP           = 34; // JMP(instruction(B))
		JMPIFTRUE     = 35; // if (A) { JMP(instruction(B)) }
		JMPIFFALSE    = 36; // if (!A) { JMP(instruction(B)) }
		JMPIFNIL      = 37; // if (A == nil) { JMP(instruction(B)) }
*/

OPCODE_FUNCTION(opjmp) {
	// this should never be called directly due to jmp patching
	throw;
}

OPCODE_FUNCTION(opjmpiffalse) {
	if (!run.state[instr.a].tobool(run.state)) {
		return instr.fastlookup[1];
	}

	return instr.fastlookup[0];
}

OPCODE_FUNCTION(opjmpiftrue) {
	if (run.state[instr.a].tobool(run.state)) {
		return instr.fastlookup[1];
	}

	return instr.fastlookup[0];
}

static bool has_init = false;

void luafunctionobject::init() {
	if (has_init) {
		return;
	}
	has_init = true;

	memset(luaopcodes, 0, sizeof(luaopcodes));

	luaopcodes[bytecode::instruction_opcode_ENVIRONMENTGET] = openvironmentget;
	luaopcodes[bytecode::instruction_opcode_NUMBER] = opnumber;
	luaopcodes[bytecode::instruction_opcode_CALL] = opcall;
	luaopcodes[bytecode::instruction_opcode_CALLM] = opcallm;
	luaopcodes[bytecode::instruction_opcode_MOV] = opmov;
	luaopcodes[bytecode::instruction_opcode_STRING] = opstring;
	luaopcodes[bytecode::instruction_opcode_INDEX] = opindex;
	luaopcodes[bytecode::instruction_opcode_GREATERTHANEQUAL] = opgreaterthanequal;
	luaopcodes[bytecode::instruction_opcode_LESSTHANEQUAL] = oplessthanequal;
	luaopcodes[bytecode::instruction_opcode_NOTEQUALS] = opnotequals;
	luaopcodes[bytecode::instruction_opcode_NOT] = opnot;
	luaopcodes[bytecode::instruction_opcode_MINUS] = opminus;
	luaopcodes[bytecode::instruction_opcode_CONSTANT] = opconstant;
	luaopcodes[bytecode::instruction_opcode_JMP] = opjmp;
	luaopcodes[bytecode::instruction_opcode_JMPIFFALSE] = opjmpiffalse;
	luaopcodes[bytecode::instruction_opcode_JMPIFTRUE] = opjmpiftrue;
	MATHOPS(OPMAPFUNC)
	COMPAREOPS(OPMAPFUNC)
}

state::_retdata luafunctionobject::call(softwarestate &state, int nrets, int nargs) {
	_running run { state, this, nrets, nargs };

	state->top = state->base + stacksize;

	instruction *instruction = allocated.get();

	while (instruction) {
		instruction = instruction->opcode(run, *instruction);
	}

	return run.retdata;
}

object luafunctionobject::create(softwarestate &state, std::shared_ptr<bytecode::prototype> proto) {
	return object(state.luafunctionallocator.take(state, proto), true);
}

luafunctionobject::luafunctionobject(softwarestate &state, std::shared_ptr<bytecode::prototype> proto) {
	auto oob = proto->instructions_size();
	allocated = std::shared_ptr<instruction>(new instruction[oob], std::default_delete<instruction[]>());

	auto instructions = allocated.get();


	for (int i = 0; i < proto->instructions_size(); i++) {
		auto &instr = proto->instructions(i);

		if (instr.op() >= bytecode::instruction_opcode_opcode_MAX) {
			throw exception("unknown opcode: " + instr.op());
		}

		instruction &generated = instructions[i];
		generated.opcode = luaopcodes[instr.op()];
		generated.a = instr.a();
		generated.b = instr.b();
		generated.c = instr.c();

		generated.fastlookup[0] = i == oob - 1 ? nullptr : &instructions[i + 1];

		if (!generated.opcode) {
			throw exception("opcode not implemented: " + bytecode::instruction_opcode_Name(instr.op()));
		}
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
			patchinstr.fastlookup[1] = patchinstr.b >= oob ? nullptr : &instructions[patchinstr.b];
		default:
			break;
		}
	}

	for (int i = 0; i < proto->instructions_size(); i++) {
		auto &patchproto = proto->instructions(i);
		auto &patchinstr = instructions[i];

		for (int i = 0; i < sizeof(luafunctionobject::instruction::fastlookup) / sizeof(luafunctionobject::instruction::fastlookup[0]); i++) {
			// fast patch jmp instructions
			while (patchinstr.fastlookup[i] && patchinstr.fastlookup[i]->opcode == opjmp) {
				patchinstr.fastlookup[i] = patchinstr.fastlookup[i]->fastlookup[1];
			}
		}
	}

	for (int i = 0; i < proto->strings_size(); i++) {
		strings.push_back(object(proto->strings(i)));
	}

	for (int i = 0; i < proto->numbers_size(); i++) {
		numbers.push_back(object(proto->numbers(i)));
	}

	stacksize = proto->stacksize();
}

luafunctionobject::luafunctionobject() { }
luafunctionobject::~luafunctionobject() { }

object functionobject::metatable(softwarestate &state) const {
	return state.function_metatable;
}

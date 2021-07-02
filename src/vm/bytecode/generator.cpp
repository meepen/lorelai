#include "generator.hpp"
#include "bytecode.hpp"
#include <deque>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::bytecode;


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::localassignmentstatement) { // TODO: VARARG
	std::deque<size_t> indexes;
	for (auto &_name : obj.left) {
		auto name = dynamic_cast<expressions::nameexpression *>(_name.get());
		if (!name) {
			throw;
		}

		indexes.push_back(curfunc.createlocal(name->name));
	}

	for (auto &_expr : obj.right) {
		size_t target = 0;
		size_t size = 0;
		if (indexes.size() > 0) {
			// we still have a local variable to assign to
			target = indexes[0];
			indexes.pop_front();
			size = 1; // TODO: vararg stuff
		}
		else {
			size = 0;
		}

		// bug: local a; a = b-a
		runexpressionhandler(_expr, target, size);
	}

	// fill the rest with nil
	for (int i = obj.right.size(); i < obj.left.size(); i++) {
		auto index = indexes[0];
		indexes.pop_front();

		emit(instruction_opcode_CONSTANT, index, 2);
	}

	return false;
}


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::assignmentstatement) { // TODO: VARARG
	for (int i = 0; i < std::max(obj.left.size(), obj.right.size()); i++) {
		if (i < obj.left.size()) {
			auto &lhs = obj.left[i];

			if (auto name = dynamic_cast<expressions::nameexpression *>(lhs.get())) {
				auto scope = curfunc.curscope->findvariablescope(name->name);
				if (scope) {
					size_t target = scope->getvariableindex(name->name);
					pushornil(obj.right, i, target);
				} // TODO: upvalues
				else {
					size_t target = curfunc.gettemp(2);

					pushornil(obj.right, i, target + 1);
					emit(instruction_opcode_ENVIRONMENTSET, target, add(name->name), target + 1);

					curfunc.freetemp(target, 2);
				}
			}
			else if (auto index = dynamic_cast<expressions::dotexpression *>(lhs.get())) {
				size_t target = curfunc.gettemp(3);
				
				runexpressionhandler(index->prefix, target, 1);
				expressions::stringexpression string(index->index->tostring());
				runexpressionhandler(string, target + 1, 1);
				pushornil(obj.right, i, target + 2);

				emit(instruction_opcode_SETINDEX, target, target + 1, target + 2);

				curfunc.freetemp(target, 3);
			}
			else if (auto index = dynamic_cast<expressions::indexexpression *>(lhs.get())) {
				size_t target = curfunc.gettemp(3);
				
				runexpressionhandler(index->prefix, target, 1);
				runexpressionhandler(index->index, target + 1, 1);
				pushornil(obj.right, i, target + 2);

				emit(instruction_opcode_SETINDEX, target, target + 1, target + 2);

				curfunc.freetemp(target, 3);
			}
			else {
				throw;
			}
		}
		else {
			runexpressionhandler(obj.right[i], 0, 0);
		}
	}

	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::ifstatement) {
	_ifqueue queue;
	queue.target = curfunc.gettemp(1);
	runexpressionhandler(obj.conditional, queue.target, 1);
	queue.patch = emit(instruction_opcode_JMPIFFALSE, queue.target);
	ifqueue.push_back(queue);

	curfunc.pushscope();
	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::elseifstatement) {
	auto &queue = ifqueue.back();
	queue.jmpends.push_back(emit(instruction_opcode_JMP, 0));

	curfunc.popscope();

	queue.patch->set_b(curfunc.proto->instructions_size());

	runexpressionhandler(obj.conditional, queue.target, 1);
	queue.patch = emit(instruction_opcode_JMPIFFALSE, queue.target);
	curfunc.pushscope();
	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::elsestatement) {
	auto &queue = ifqueue.back();
	queue.jmpends.push_back(emit(instruction_opcode_JMP, 0));

	curfunc.popscope();

	queue.patch->set_b(curfunc.proto->instructions_size());
	queue.patch = nullptr;

	curfunc.pushscope();
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::ifstatement) {
	curfunc.popscope();
	auto &queue = ifqueue.back();
	if (queue.patch) {
		queue.patch->set_b(curfunc.proto->instructions_size());
	}
	curfunc.freetemp(queue.target, 1);

	for (auto &jmpend : queue.jmpends) {
		jmpend->set_b(curfunc.proto->instructions_size());
	}

	ifqueue.pop_back();
	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::whilestatement) {
	_loopqueue data;
	data.startinstr = curfunc.proto->instructions_size();
	data.stackreserved = curfunc.gettemp(1);

	runexpressionhandler(obj.conditional, data.stackreserved, 1);
	data.patches.push_back(emit(instruction_opcode_JMPIFFALSE, data.stackreserved));

	loopqueue.push_back(data);
	curfunc.pushscope();
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::whilestatement) {
	auto data = loopqueue.back();

	emit(instruction_opcode_JMP, 0, data.startinstr);

	for (auto &patch : data.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}

	curfunc.freetemp(data.stackreserved, 1);
	curfunc.popscope();
	loopqueue.pop_back();
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::repeatstatement) {
	auto data = loopqueue.back();

	auto target = curfunc.gettemp(1);

	runexpressionhandler(obj.conditional, target, 1);
	emit(instruction_opcode_JMPIFFALSE, target, data.startinstr);
	
	for (auto &patch : data.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}

	curfunc.freetemp(target, 1);

	loopqueue.pop_back();
	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::breakstatement) {
	if (loopqueue.size() == 0) {
		throw;
	}
	loopqueue.back().patches.push_back(emit(instruction_opcode_JMP, 0));

	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::forinstatement) {
	_loopqueue queue;

	curfunc.pushscope();
	queue.stackreserved = curfunc.gettemp(3);
	queue.extrastack = curfunc.gettemp(std::max((size_t)3, obj.iternames.size()));
	for (int i = 0; i < obj.iternames.size(); i++) {
		curfunc.curscope->addvariable(obj.iternames[i]->tostring(), queue.extrastack + i);
	}

	// loop prep: local f, s, v = inexprs
	for (int i = 0; i < obj.inexprs.size(); i++) {
		auto &inexpr = obj.inexprs[i];
		size_t amount;
		if (i == obj.inexprs.size() - 1 && i < 3) {
			amount = 3 - i;
		}
		else {
			amount = i >= 3 ? 0 : 1;
		}

		runexpressionhandler(inexpr, queue.extrastack + 3 - amount, amount);
	}

	// begin loop
	queue.startinstr = curfunc.proto->instructions_size();

	emit(bytecode::instruction_opcode_MOV, queue.extrastack, queue.stackreserved, 3);
	emit(bytecode::instruction_opcode_CALL, queue.extrastack, 3, obj.iternames.size() + 1);
	queue.patches.push_back(emit(bytecode::instruction_opcode_JMPIFNIL, queue.extrastack));

	loopqueue.push_back(queue);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::forinstatement) {
	auto &queue = loopqueue.back();

	curfunc.freetemp(queue.stackreserved, 3);
	curfunc.freetemp(queue.extrastack, std::max((size_t)3, obj.iternames.size()));

	emit(bytecode::instruction_opcode_JMP, 0, queue.startinstr);

	for (auto &patch : queue.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}

	// before popping we must delete references to extrastack in the variable list to prevent double free
	auto start = queue.extrastack;
	auto ends = start + std::max((size_t)3, obj.iternames.size());
	for (int i = 0; i < obj.iternames.size(); i++) {
		auto name = obj.iternames[i]->tostring();
		auto index = curfunc.curscope->getvariableindex(name);
		if (index >= start && index < ends) {
			curfunc.curscope->variables.erase(name);
		}
	}

	loopqueue.pop_back();
	curfunc.popscope();
	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::fornumstatement) {
	_loopqueue queue;
	queue.stackreserved = curfunc.gettemp(3); // var, limit, step
	runexpressionhandler(obj.startexpr, queue.stackreserved, 1);
	runexpressionhandler(obj.endexpr, queue.stackreserved + 1, 1);
	if (obj.stepexpr) {
		runexpressionhandler(obj.stepexpr, queue.stackreserved + 2, 1);
	}
	else {
		expressions::numberexpression data(1.0);
		runexpressionhandler(data, queue.stackreserved + 2, 1);
	}

	// start loop

	queue.startinstr = curfunc.proto->instructions_size();

	queue.patches.push_back(emit(bytecode::instruction_opcode_FORCHECK, queue.stackreserved));
	curfunc.pushscope();

	emit(bytecode::instruction_opcode_MOV, curfunc.createlocal(obj.itername->tostring()), queue.stackreserved, 1);

	// start body

	loopqueue.push_back(queue);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::fornumstatement) {
	auto &queue = loopqueue.back();

	emit(bytecode::instruction_opcode_ADD, queue.stackreserved, queue.stackreserved, queue.stackreserved + 2);
	emit(bytecode::instruction_opcode_JMP, 0, queue.startinstr);

	for (auto &patch : queue.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}

	curfunc.freetemp(queue.stackreserved, 3);
	curfunc.popscope();
	loopqueue.pop_back();
	return false;
}


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::returnstatement) {
	auto realrets = obj.children.size();
	auto rets = realrets;
	auto target = curfunc.gettemp(realrets);
	size_t varargtype = 0;
	for (int i = 0; i < obj.children.size(); i++) {
		auto &child = obj.children[i];

		if (i == rets - 1 && dynamic_cast<expressions::varargexpression *>(child.get())) {
			rets--;
			varargtype = 2;
		}
		else if (i == rets - 1 && dynamic_cast<expressions::functioncallexpression *>(child.get())) {
			rets--;
			varargtype = 1;
			runexpressionhandler(child, -1, 0);
		}
		else {
			runexpressionhandler(child, target + i, 1);
		}
	}

	emit(bytecode::instruction_opcode_RETURN, target, rets, varargtype);
	
	curfunc.freetemp(target, rets);
	return false;
}

void bytecodegenerator::pushornil(std::vector<std::shared_ptr<lorelai::parser::node>> &v, int index, size_t target) {
	if (index < v.size()) {
		runexpressionhandler(v[index], target, 1);
	}
	else {
		emit(bytecode::instruction_opcode_CONSTANT, target, 2);
	}
}

instruction *bytecodegenerator::emit(instruction_opcode opcode) {
	auto instruction = curfunc.proto->add_instructions();
	instruction->set_op(opcode);
	return instruction;
}

instruction *bytecodegenerator::emit(instruction_opcode opcode, std::uint32_t a) {
	auto instruction = curfunc.proto->add_instructions();
	instruction->set_op(opcode);
	instruction->set_a(a);
	return instruction;
}

instruction *bytecodegenerator::emit(instruction_opcode opcode, std::uint32_t a, std::uint32_t b) {
	auto instruction = curfunc.proto->add_instructions();
	instruction->set_op(opcode);
	instruction->set_a(a);
	instruction->set_b(b);
	return instruction;
}

instruction *bytecodegenerator::emit(instruction_opcode opcode, std::uint32_t a, std::uint32_t b, std::uint32_t c) {
	auto instruction = curfunc.proto->add_instructions();
	instruction->set_op(opcode);
	instruction->set_a(a);
	instruction->set_b(b);
	instruction->set_c(c);
	return instruction;
}
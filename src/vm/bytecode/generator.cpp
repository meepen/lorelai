#include "generator.hpp"
#include "bytecode.hpp"
#include <deque>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::bytecode;


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::localassignmentstatement) { // TODO: VARARG
	variablevisitor::visit(obj, container);

	auto size = std::min(obj.right.size(), obj.left.size());
	_assignmentqueue queue { curfunc.getslots(size), static_cast<std::uint32_t>(size) };

	std::uint32_t target = queue.index;

	for (auto &_expr : obj.right) {
		size_t size = 0;
		if (obj.left.size() > 0) {
			// we still have a local variable to assign to
			size = 1; // TODO: vararg stuff
		}
		else {
			size = 0;
		}

		runexpressionhandler(_expr, target, size);
		target++;
	}

	assignmentqueue.push_back(queue);

	return false;
}



LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::localassignmentstatement) {
	variablevisitor::postvisit(obj, container);
	auto queue = assignmentqueue.back();
	assignmentqueue.pop_back();
	// fill the rest with nil
	auto minsize = std::min((size_t)queue.size, obj.left.size());
	for (std::uint32_t i = 0; i < minsize; i++) {
		mov(curfunc.varlookup[obj.left[i]], queue.index + i, 1);
	}
	for (auto i = minsize + 1; i < obj.left.size(); i++) {
		emit(instruction_opcode_CONSTANT, curfunc.varlookup[obj.left[i]], 2);
	}
}


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::assignmentstatement) { // TODO: VARARG
	variablevisitor::visit(obj, container);

	for (int i = 0; i < std::max(obj.left.size(), obj.right.size()); i++) {
		if (i < obj.left.size()) {
			auto lhs = obj.left[i];

			if (auto name = dynamic_cast<expressions::nameexpression *>(lhs)) {
				if (curfunc.hasvariable(name->name)) {
					pushornil(obj.right, i, curfunc.varlookup[name->name]);
				} // TODO: upvalues
				else {
					auto target = curfunc.getslots(2);

					pushornil(obj.right, i, target + 1);
					emit(instruction_opcode_ENVIRONMENTSET, target, add(name->name), target + 1);

					curfunc.freeslots(target, 2);
				}
			}
			else if (auto index = dynamic_cast<expressions::dotexpression *>(lhs)) {
				auto target = curfunc.getslots(3);
				
				runexpressionhandler(index->prefix, target, 1);
				expressions::stringexpression indexstr(index->index);
				runexpressionhandler(&indexstr, target + 1, 1);
				pushornil(obj.right, i, target + 2);

				emit(instruction_opcode_SETINDEX, target, target + 1, target + 2);

				curfunc.freeslots(target, 3);
			}
			else if (auto index = dynamic_cast<expressions::indexexpression *>(lhs)) {
				auto target = curfunc.getslots(3);
				
				runexpressionhandler(index->prefix, target, 1);
				runexpressionhandler(index->index, target + 1, 1);
				pushornil(obj.right, i, target + 2);

				emit(instruction_opcode_SETINDEX, target, target + 1, target + 2);

				curfunc.freeslots(target, 3);
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
	variablevisitor::visit(obj, container);

	_ifqueue queue;
	queue.target = curfunc.getslots(1);
	runexpressionhandler(obj.conditional, queue.target, 1);
	queue.patch = emit(instruction_opcode_JMPIFFALSE, queue.target);
	ifqueue.push_back(queue);

	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::elseifstatement) {
	variablevisitor::visit(obj, container);
	
	auto &queue = ifqueue.back();
	queue.jmpends.push_back(emit(instruction_opcode_JMP, 0));

	queue.patch->set_b(curfunc.proto->instructions_size());

	runexpressionhandler(obj.conditional, queue.target, 1);
	queue.patch = emit(instruction_opcode_JMPIFFALSE, queue.target);
	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::elsestatement) {
	variablevisitor::visit(obj, container);

	auto &queue = ifqueue.back();
	queue.jmpends.push_back(emit(instruction_opcode_JMP, 0));

	queue.patch->set_b(curfunc.proto->instructions_size());
	queue.patch = nullptr;
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::ifstatement) {
	variablevisitor::visit(obj, container);

	auto &queue = ifqueue.back();
	if (queue.patch) {
		queue.patch->set_b(curfunc.proto->instructions_size());
	}
	curfunc.freeslots(queue.target, 1);

	for (auto &jmpend : queue.jmpends) {
		jmpend->set_b(curfunc.proto->instructions_size());
	}

	ifqueue.pop_back();
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::whilestatement) {
	variablevisitor::visit(obj, container);

	_loopqueue data;
	data.startinstr = curfunc.proto->instructions_size();
	data.stackreserved = curfunc.getslots(1);

	runexpressionhandler(obj.conditional, data.stackreserved, 1);
	data.patches.push_back(emit(instruction_opcode_JMPIFFALSE, data.stackreserved));

	loopqueue.push_back(data);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::whilestatement) {
	variablevisitor::postvisit(obj, container);

	auto data = loopqueue.back();

	emit(instruction_opcode_JMP, 0, data.startinstr);

	for (auto &patch : data.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}

	curfunc.freeslots(data.stackreserved, 1);
	loopqueue.pop_back();
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::repeatstatement) {
	variablevisitor::postvisit(obj, container);

	auto data = loopqueue.back();

	auto target = curfunc.getslots(1);

	runexpressionhandler(obj.conditional, target, 1);
	emit(instruction_opcode_JMPIFFALSE, target, data.startinstr);
	
	for (auto &patch : data.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}

	curfunc.freeslots(target, 1);

	loopqueue.pop_back();
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::breakstatement) {
	if (loopqueue.size() == 0) {
		throw;
	}
	loopqueue.back().patches.push_back(emit(instruction_opcode_JMP, 0));

	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::forinstatement) {
	/* TODO: ALL OF THIS IS WRONG SINCE UPDATE */
	throw exception("for in is not implemented");
	variablevisitor::visit(obj, container);

	_loopqueue queue;

	queue.stackreserved = curfunc.getslots(3);


	// begin loop
	queue.startinstr = curfunc.proto->instructions_size();

	mov(queue.extrastack, queue.stackreserved, 3);
	emit(bytecode::instruction_opcode_CALL, queue.extrastack, 3, obj.iternames.size() + 1);
	queue.patches.push_back(emit(bytecode::instruction_opcode_JMPIFNIL, queue.extrastack));

	loopqueue.push_back(queue);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::forinstatement) {
	variablevisitor::postvisit(obj, container);

	auto &queue = loopqueue.back();

	curfunc.freeslots(queue.stackreserved, 3);
	curfunc.freeslots(queue.extrastack, std::max((size_t)3, obj.iternames.size()));

	emit(bytecode::instruction_opcode_JMP, 0, queue.startinstr);

	for (auto &patch : queue.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}
	loopqueue.pop_back();
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::fornumstatement) {
	variablevisitor::visit(obj, container);

	_loopqueue queue;
	queue.stackreserved = curfunc.getslots(3); // var, limit, step
	runexpressionhandler(obj.startexpr, queue.stackreserved, 1);
	runexpressionhandler(obj.endexpr, queue.stackreserved + 1, 1);
	if (obj.stepexpr) {
		runexpressionhandler(obj.stepexpr, queue.stackreserved + 2, 1);
	}
	else {
		expressions::numberexpression num(1.0);
		runexpressionhandler(&num, queue.stackreserved + 2, 1);
	}

	// start loop

	queue.startinstr = curfunc.proto->instructions_size();

	queue.patches.push_back(emit(bytecode::instruction_opcode_FORCHECK, queue.stackreserved));

	mov(curfunc.varlookup[obj.itername], queue.stackreserved, 1);

	// start body
	loopqueue.push_back(queue);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::fornumstatement) {
	variablevisitor::postvisit(obj, container);

	auto &queue = loopqueue.back();

	emit(bytecode::instruction_opcode_ADD, queue.stackreserved, queue.stackreserved, queue.stackreserved + 2);
	emit(bytecode::instruction_opcode_JMP, 0, queue.startinstr);

	for (auto &patch : queue.patches) {
		patch->set_b(curfunc.proto->instructions_size());
	}

	curfunc.freeslots(queue.stackreserved, 3);
	loopqueue.pop_back();
}


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::returnstatement) {
	auto realrets = obj.retlist.size();
	auto rets = realrets;
	auto target = curfunc.getslots(realrets);
	std::uint32_t varargtype = 0;
	for (int i = 0; i < obj.retlist.size(); i++) {
		auto &child = obj.retlist[i];

		if (i == rets - 1 && dynamic_cast<expressions::varargexpression *>(child)) {
			rets--;
			varargtype = 2;
		}
		else if (i == rets - 1 && dynamic_cast<expressions::functioncallexpression *>(child)) {
			rets--;
			varargtype = 1;
			runexpressionhandler(child, -1, 0);
		}
		else {
			runexpressionhandler(child, target + i, 1);
		}
	}

	emit(bytecode::instruction_opcode_RETURN, target, rets, varargtype);
	
	curfunc.freeslots(target, rets);
	return false;
}

void bytecodegenerator::pushornil(std::vector<lorelai::parser::node *> &v, int index, std::uint32_t target) {
	if (index < v.size()) {
		runexpressionhandler(v[index], target, 1);
	}
	else {
		emit(bytecode::instruction_opcode_CONSTANT, target, 2);
	}
}

instruction *bytecodegenerator::emit(instruction_opcode opcode, std::uint32_t a, std::uint32_t b, std::uint32_t c) {
	auto instruction = curfunc.proto->add_instructions();
	instruction->set_op(opcode);
	instruction->set_a(a);
	instruction->set_b(b);
	instruction->set_c(c);
	return instruction;
}
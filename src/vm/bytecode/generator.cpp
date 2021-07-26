#include "generator.hpp"
#include "bytecode.hpp"
#include "bcconstants.hpp"
#include <deque>

using namespace lorelai;
using namespace lorelai::parser;
using namespace lorelai::bytecode;

parser::node *bytecodegenerator::trycollapse(parser::node *expr) {
	auto collapsed = collapseconstant(*this, *expr);
	if (!collapsed) {
		return expr;
	}

	allocatedconsts.push_back(collapsed);
	return collapsed;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::localassignmentstatement) {
	auto fullscope = variablefinder->scopemap[container].get();

	size_t constants = 0;
	std::vector<variable> lhs;
	std::vector<parser::node *> rhs;

	for (size_t i = 0; i < std::min(obj.right.size(), obj.left.size()); i++) {
		auto n = obj.left[i];
		// problem: multiple variables with same name
		auto found = fullscope->find(n, false, curscope->versionof(n) + 1);
		if (!found) {
			throw exception("couldn't find future variable");
		}

		if (isconstant(*found)) {
			auto expr = obj.right[i];
			// TODO: why is this slower?
			constantmap[*found] = trycollapse(expr);
			constants++;
			continue;
		}

		lhs.push_back(*found);
		rhs.push_back(obj.right[i]);
	}

	for (size_t i = std::min(obj.right.size(), obj.left.size()); i < obj.right.size(); i++) {
		rhs.push_back(obj.right[i]);	
	}


	auto size = obj.right.size() - constants;
	auto slots = funcptr->getslots(size);
	auto current = slots;

	for (size_t i = 0; i < rhs.size(); i++) {
		std::uint32_t target = obj.left.size() < i ? 0 : current++;

		runexpressionhandler(rhs[i], target, 1);
	}

	variablevisitor::postvisit(obj, container);

	for (std::uint32_t i = 0; i < lhs.size(); i++) {
		auto varid = funcptr->varlookup[lhs[i].name];
		if (rhs.size() < i) {
			emit(prototype::OP_CONSTANT, varid, 2);
		}
		else {
			mov(varid, slots + i);
		}
	}

	funcptr->freeslots(slots, size);
}


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::assignmentstatement) { // TODO: VARARG
	variablevisitor::visit(obj, container);

	for (int i = 0; i < std::max(obj.left.size(), obj.right.size()); i++) {
		if (i < obj.left.size()) {
			auto lhs = obj.left[i];

			if (auto name = dynamic_cast<expressions::nameexpression *>(lhs)) {
				if (funcptr->hasvariable(name->name)) {
					pushornil(obj.right, i, funcptr->varlookup[name->name]);
				} // TODO: upvalues
				else {
					auto target = funcptr->getslots(2);

					pushornil(obj.right, i, target + 1);
					emit(prototype::OP_ENVIRONMENTSET, target, add(name->name), target + 1);

					funcptr->freeslots(target, 2);
				}
			}
			else if (auto index = dynamic_cast<expressions::dotexpression *>(lhs)) {
				auto target = funcptr->getslots(3);
				
				runexpressionhandler(index->prefix, target, 1);
				expressions::stringexpression indexstr(index->index);
				runexpressionhandler(&indexstr, target + 1, 1);
				pushornil(obj.right, i, target + 2);

				emit(prototype::OP_SETINDEX, target, target + 1, target + 2);

				funcptr->freeslots(target, 3);
			}
			else if (auto index = dynamic_cast<expressions::indexexpression *>(lhs)) {
				auto target = funcptr->getslots(3);
				
				runexpressionhandler(index->prefix, target, 1);
				runexpressionhandler(index->index, target + 1, 1);
				pushornil(obj.right, i, target + 2);

				emit(prototype::OP_SETINDEX, target, target + 1, target + 2);

				funcptr->freeslots(target, 3);
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
	queue.target = funcptr->getslots(1);
	runexpressionhandler(obj.conditional, queue.target, 1);
	queue.patch = emit(prototype::OP_JMPIFFALSE, queue.target);
	ifqueue.push_back(queue);

	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::elseifstatement) {
	variablevisitor::visit(obj, container);
	
	auto &queue = ifqueue.back();
	queue.jmpends.push_back(emit(prototype::OP_JMP, 0));

	if (queue.patch) {
		(*queue.patch)->set_b(funcptr->proto->instructions_size());
	}

	runexpressionhandler(obj.conditional, queue.target, 1);
	queue.patch = emit(prototype::OP_JMPIFFALSE, queue.target);
	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::elsestatement) {
	variablevisitor::visit(obj, container);

	auto &queue = ifqueue.back();
	queue.jmpends.push_back(emit(prototype::OP_JMP, 0));

	if (queue.patch) {
		(*queue.patch)->set_b(funcptr->proto->instructions_size());
		queue.patch = {};
	}

	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::ifstatement) {
	variablevisitor::postvisit(obj, container);

	auto &queue = ifqueue.back();
	if (queue.patch) {
		(*queue.patch)->set_b(funcptr->proto->instructions_size());
	}
	funcptr->freeslots(queue.target, 1);

	for (auto &jmpend : queue.jmpends) {
		jmpend->set_b(funcptr->proto->instructions_size());
	}

	ifqueue.pop_back();
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::whilestatement) {
	variablevisitor::visit(obj, container);

	_loopqueue data;
	data.startinstr = funcptr->proto->instructions_size();
	data.stackreserved = funcptr->getslots(1);

	runexpressionhandler(obj.conditional, data.stackreserved, 1);
	data.patches.push_back(emit(prototype::OP_JMPIFFALSE, data.stackreserved));

	loopqueue.push_back(data);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::whilestatement) {
	variablevisitor::postvisit(obj, container);

	auto data = loopqueue.back();

	emit(prototype::OP_JMP, 0, data.startinstr);

	for (auto &patch : data.patches) {
		patch->set_b(funcptr->proto->instructions_size());
	}

	funcptr->freeslots(data.stackreserved, 1);
	loopqueue.pop_back();
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::repeatstatement) {
	variablevisitor::postvisit(obj, container);

	auto data = loopqueue.back();

	auto target = funcptr->getslots(1);

	runexpressionhandler(obj.conditional, target, 1);
	emit(prototype::OP_JMPIFFALSE, target, data.startinstr);
	
	for (auto &patch : data.patches) {
		patch->set_b(funcptr->proto->instructions_size());
	}

	funcptr->freeslots(target, 1);

	loopqueue.pop_back();
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::breakstatement) {
	if (loopqueue.size() == 0) {
		throw;
	}
	loopqueue.back().patches.push_back(emit(prototype::OP_JMP, 0));

	return false;
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::forinstatement) {
	/* TODO: ALL OF THIS IS WRONG SINCE UPDATE */
	throw exception("for in is not implemented");
	variablevisitor::visit(obj, container);

	_loopqueue queue;

	queue.stackreserved = funcptr->getslots(3);


	// begin loop
	queue.startinstr = funcptr->proto->instructions_size();

	mov(queue.extrastack, queue.stackreserved, 3);
	emit(bytecode::prototype::OP_CALL, queue.extrastack, 3, obj.iternames.size() + 1);
	queue.patches.push_back(emit(bytecode::prototype::OP_JMPIFNIL, queue.extrastack));

	loopqueue.push_back(queue);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::forinstatement) {
	variablevisitor::postvisit(obj, container);

	auto &queue = loopqueue.back();

	funcptr->freeslots(queue.stackreserved, 3);
	funcptr->freeslots(queue.extrastack, std::max((size_t)3, obj.iternames.size()));

	emit(bytecode::prototype::OP_JMP, 0, queue.startinstr);

	for (auto &patch : queue.patches) {
		patch->set_b(funcptr->proto->instructions_size());
	}
	loopqueue.pop_back();
}

LORELAI_VISIT_DEFINE(bytecodegenerator, statements::fornumstatement) {
	variablevisitor::visit(obj, container);

	_loopqueue queue;
	queue.stackreserved = funcptr->getslots(3); // var, limit, step
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

	queue.startinstr = funcptr->proto->instructions_size();

	queue.patches.push_back(emit(bytecode::prototype::OP_FORCHECK, queue.stackreserved));

	mov(funcptr->varlookup[obj.itername], queue.stackreserved, 1);

	// start body
	loopqueue.push_back(queue);
	return false;
}

LORELAI_POSTVISIT_DEFINE(bytecodegenerator, statements::fornumstatement) {
	variablevisitor::postvisit(obj, container);

	auto &queue = loopqueue.back();

	emit(bytecode::prototype::OP_ADD, queue.stackreserved, queue.stackreserved, queue.stackreserved + 2);
	emit(bytecode::prototype::OP_JMP, 0, queue.startinstr);

	for (auto &patch : queue.patches) {
		patch->set_b(funcptr->proto->instructions_size());
	}

	funcptr->freeslots(queue.stackreserved, 3);
	loopqueue.pop_back();
}


LORELAI_VISIT_DEFINE(bytecodegenerator, statements::returnstatement) {
	auto realrets = obj.retlist.size();
	auto rets = realrets;
	auto target = funcptr->getslots(realrets);
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

	emit(bytecode::prototype::OP_RETURN, target, rets, varargtype);
	
	funcptr->freeslots(target, rets);
	return false;
}

void bytecodegenerator::pushornil(std::vector<lorelai::parser::node *> &v, int index, std::uint32_t target) {
	if (index < v.size()) {
		runexpressionhandler(v[index], target, 1);
	}
	else {
		emit(bytecode::prototype::OP_CONSTANT, target, 2);
	}
}

prototype::instruct_ptr bytecodegenerator::emit(prototype::_opcode opcode, std::uint8_t a, std::uint8_t b, std::uint8_t c) {
	return funcptr->proto->addinstruction(opcode, a, b, c);
}
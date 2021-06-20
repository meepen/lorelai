#include "bytecode.hpp"
#include "visitor.hpp"
#include "statements.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_set>

using namespace lorelai;
using namespace lorelai::vm;

using namespace lorelai::parser;

/*
#define LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(lorelai::parser::statements::returnstatement) \
	fn(lorelai::parser::statements::dostatement) \
	fn(lorelai::parser::statements::whilestatement) \
	fn(lorelai::parser::statements::repeatstatement) \
	fn(lorelai::parser::statements::localassignmentstatement) \
	fn(lorelai::parser::statements::localfunctionstatement) \
	fn(lorelai::parser::statements::fornumstatement) \
	fn(lorelai::parser::statements::forinstatement) \
	fn(lorelai::parser::statements::ifstatement) \
	fn(lorelai::parser::statements::functionstatement) \
	fn(lorelai::parser::statements::functioncallstatement) \
	fn(lorelai::parser::statements::assignmentstatement)

#define LORELAI_STATEMENT_CLASS_MACRO(fn) \
	LORELAI_STATEMENT_BRANCH_CLASS_MACRO(fn) \
	fn(lorelai::parser::statements::breakstatement)
*/

class scope {
public:

	scope *findvariablescope(string name) {
		auto var = variables.find(name);
		if (var != variables.end()) {
			return this;
		}

		if (parent) {
			return parent->findvariablescope(name);
		}

		return nullptr;
	}

	size_t addvariable(string name) {
		variables[name] = locals;
		return locals++;
	}

	size_t getvariableindex(string name) {
		auto var = variables.find(name);
		if (var != variables.end()) {
			return var->second;
		}

		return -1;
	}

	size_t hasvariable(string name) {
		return variables.find(name) != variables.end();
	}

public:
	std::unordered_map<string, size_t> variables;
	size_t locals = 0;
	std::shared_ptr<scope> parent;
};

class bytecodegenerator : public visitor {
public:
	using visitor::visit;

	LORELAI_VISIT_FUNCTION(statements::localassignmentstatement) {
		std::vector<size_t> indexes;
		for (auto &_name : obj.left) {
			auto name = dynamic_cast<expressions::nameexpression *>(_name.get());
			if (!name) {
				throw;
			}

			indexes.push_back(curscope->addvariable(name->name));
		}
		std::cout << "got local assignment statement" << std::endl;
		return false;
	}

public:
	std::shared_ptr<scope> curscope = std::make_shared<scope>();
	bytecode::prototype proto;
};


bytecode::prototype lorelai::vm::parse(chunk &data) {
	bytecodegenerator generator;
	std::shared_ptr<node> container = std::make_shared<chunk>(data);

	data.accept(generator, container);

	return generator.proto;
}

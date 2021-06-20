#include "bytecode.hpp"
#include "visitor.hpp"
#include "statements.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <deque>
#include <unordered_set>
#include <typeindex>

using namespace lorelai;
using namespace lorelai::vm;

using namespace lorelai::parser;


#ifdef __linux__
#include <cxxabi.h>
template <typename T>
std::string gettypename(T &data) {
    auto ptr = std::unique_ptr<char, decltype(& std::free)>{
        abi::__cxa_demangle(typeid(data).name(), nullptr, nullptr, nullptr),
        std::free
    };
    return {ptr.get()};
}
#else
template <typename T>
std::string gettypename(T &data) {
	return typeid(data).name();
}
#endif

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

class stack {
public:
	size_t getslot() {
		size_t ret;
		if (freeslots.size() > 0) {
			ret = freeslots[freeslots.size() - 1];
			freeslots.pop_back();
		}
		else {
			ret = maxsize++;
		}

		return ret;
	}

	void freeslot(size_t slot) {
		freeslots.push_back(slot);
	}

public:
	size_t maxsize;
	std::vector<size_t> freeslots;
};

class scope {
public:
	scope *findvariablescope(string name, std::shared_ptr<scope> highest = nullptr) {
		auto var = variables.find(name);
		if (var != variables.end()) {
			return this;
		}

		if (parent && parent != highest) {
			return parent->findvariablescope(name);
		}

		return nullptr;
	}

	size_t addvariable(string name, size_t stackpos) {
		variables[name] = stackpos;
		return stackpos;
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
	std::shared_ptr<scope> parent;
};

class function {
public:
	function() {}
	function(std::shared_ptr<function> &_parent) : parent(_parent), parentscope(_parent->curscope) { }

	std::shared_ptr<scope> &pushscope() {
		auto newscope = std::make_shared<scope>();
		newscope->parent = curscope;
		curscope = newscope;
		return curscope;
	}

	std::shared_ptr<scope> &popscope() {
		if (!curscope) {
			throw;
		}

		for (auto stackpos : curscope->variables) {
			// erase stack usage for scope now that is gone
			funcstack.freeslot(stackpos.second);
		}

		curscope = curscope->parent;
		return curscope;
	}

	bool haslocal(string name) {
		return curscope->findvariablescope(name, firstscope) != nullptr;
	}

	bool hasupvalue(string name) {
		return parentscope && parentscope->hasvariable(name) || parent && parent->hasupvalue(name);
	}

	// finds a local variable and returns a stack position
	size_t findlocal(string name) {
		auto found = curscope->findvariablescope(name, firstscope);
		if (!found) {
			// not in current scope, what do?
			// return -1 for now to signify it's not here i guess??
			return -1;
		}

		return found->getvariableindex(name);
	}

	size_t createlocal(string name) {
		auto scopeindex = curscope->addvariable(name, funcstack.getslot());
		return scopeindex;
	}

	size_t gettemp() {
		return funcstack.getslot();
	}

	void freetemp(size_t slot) {
		funcstack.freeslot(slot);
	}

	// returns pair<upvalueprotoid, stackpos>
	std::pair<size_t, size_t> findupvalue(string name) {

	}

public:
	stack funcstack;
	std::shared_ptr<function> parent = nullptr;
	std::shared_ptr<scope> curscope = std::make_shared<scope>();
	std::shared_ptr<scope> firstscope = curscope;
	std::shared_ptr<scope> parentscope = nullptr;
};

class bytecodegenerator;
using expressiongenerator = void (*)(bytecodegenerator &gen, node &expr, size_t stackindex);

extern std::unordered_map<std::type_index, expressiongenerator> expressionmap;

class bytecodegenerator : public visitor {
public:
	using visitor::visit;

	LORELAI_VISIT_FUNCTION(statements::localassignmentstatement) {
		std::deque<size_t> indexes;
		for (auto &_name : obj.left) {
			auto name = dynamic_cast<expressions::nameexpression *>(_name.get());
			if (!name) {
				throw;
			}

			indexes.push_back(curfunc.createlocal(name->name));
		}

		for (auto &_expr : obj.right) {
			// generate expression bytecode NOW
			auto found = expressionmap.find(typeid(*_expr.get()));

			if (found == expressionmap.end()) {
				std::cerr << "Unsupported expression when generating localassignmentstatement: " << gettypename(*_expr.get()) << std::endl;
				throw;
			}

			auto start = proto.instructions_size();

			size_t target;
			bool is_temp = false;
			if (indexes.size() > 0) {
				// we still have a local variable to assign to
				target = indexes[0];
				indexes.pop_front();
			}
			else {
				is_temp = true;
				target = curfunc.gettemp();
			}

			found->second(*this, *_expr.get(), target);

			if (is_temp) {
				curfunc.freetemp(target);
			}
		}

		return false;
	}

public:
	void emit(bytecode::instruction_opcode opcode) {
		auto instruction = proto.add_instructions();
		instruction->set_op(opcode);
	}
	void emit(bytecode::instruction_opcode opcode, std::uint32_t a) {
		auto instruction = proto.add_instructions();
		instruction->set_op(opcode);
		instruction->set_a(a);
	}
	void emit(bytecode::instruction_opcode opcode, std::uint32_t a, std::uint32_t b) {
		auto instruction = proto.add_instructions();
		instruction->set_op(opcode);
		instruction->set_a(a);
		instruction->set_b(b);
	}
	void emit(bytecode::instruction_opcode opcode, std::uint32_t a, std::uint32_t b, std::uint32_t c) {
		auto instruction = proto.add_instructions();
		instruction->set_op(opcode);
		instruction->set_a(a);
		instruction->set_b(b);
		instruction->set_c(c);
	}

public:
	function curfunc;
	bytecode::prototype proto;
};

static void generate_numberexpression(bytecodegenerator &gen, node &expr, size_t target) {
	gen.emit(bytecode::instruction_opcode_SET, target, 1, gen.proto.numbers_size());
	gen.proto.add_numbers(dynamic_cast<expressions::numberexpression *>(&expr)->data);
}


std::unordered_map<std::type_index, expressiongenerator> expressionmap = {
	{ typeid(expressions::numberexpression), generate_numberexpression }
};


bytecode::prototype lorelai::vm::parse(chunk &data) {
	bytecodegenerator generator;
	std::shared_ptr<node> container = std::make_shared<chunk>(data);

	data.accept(generator, container);

	return generator.proto;
}

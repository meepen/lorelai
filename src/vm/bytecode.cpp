#include "bytecode.hpp"
#include "visitor.hpp"
#include "statements.hpp"
#include "args.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <deque>
#include <list>
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
	size_t getslots(size_t amount = 1) {
		bool found = false;
		for (size_t index = 0; index <= maxsize; index++) {
			found = true;
			for (size_t i = 0; i < amount; i++) {
				if (isslotinuse(index + i)) {
					found = false;
					break;
				}
			}

			if (found) {
				// obtain and return
				for (size_t i = 0; i < amount; i++) {
					if (index + i < maxsize) {
						unusedslots.remove(index + i);
					}
				}

				if (index + amount > maxsize) {
					maxsize = index + amount;
				}

				return index;
			}
		}

		throw;
	}

	void freeslots(size_t slot, size_t amount = 1) {
		for (size_t i = 0; i < amount; i++) {
			unusedslots.insert(std::lower_bound(unusedslots.begin(), unusedslots.end(), slot + i), slot + i);
		}
	}

	bool isslotinuse(size_t slot) {
		if (slot >= maxsize) {
			return false;
		}

		auto it = std::lower_bound(unusedslots.begin(), unusedslots.end(), slot);

		return it == unusedslots.end() || *it != slot;
	}

public:
	size_t maxsize = 0;
	std::list<size_t> unusedslots;
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
			funcstack.freeslots(stackpos.second);
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
		auto scopeindex = curscope->addvariable(name, funcstack.getslots());
		return scopeindex;
	}

	size_t gettemp(size_t amount = 1) {
		return funcstack.getslots(amount);
	}

	void freetemp(size_t slot, size_t amount = 1) {
		funcstack.freeslots(slot);
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
using expressiongenerator = void (*)(bytecodegenerator &gen, node &expr, size_t stackindex, size_t size);

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
			auto start = proto.instructions_size();

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

			runexpressionhandler(_expr, target, size);
		}

		// fill the rest with nil
		for (int i = obj.right.size(); i < obj.left.size(); i++) {
			auto index = indexes[0];
			indexes.pop_front();

			emit(bytecode::instruction_opcode_CONSTANT, index, 2);
		}

		return false;
	}

	LORELAI_VISIT_FUNCTION(statements::assignmentstatement) {
		for (int i = 0; i < std::max(obj.left.size(), obj.right.size()); i++) {

			if (i < obj.left.size()) {
				auto &lhs = obj.left[i];

				auto name = dynamic_cast<expressions::nameexpression *>(lhs.get());
				if (name) {
					auto scope = curfunc.curscope->findvariablescope(name->name);
					if (scope) {
						size_t target = scope->getvariableindex(name->name);
						pushornil(obj.right, i, target);
					} // TODO: upvalues
					else {
						size_t target = curfunc.gettemp(3);

						emit(bytecode::instruction_opcode_ENVIRONMENT, target);
						expressions::stringexpression string(name->name);
						runexpressionhandler(string, target + 1, 1);
						pushornil(obj.right, i, target + 2);

						emit(bytecode::instruction_opcode_SETINDEX, target, target + 1, target + 2);

						curfunc.freetemp(target);
					}
				}
				else {
					auto index = dynamic_cast<expressions::indexexpression *>(lhs.get());
					throw;
					// TODO
				}
			}
		}

		return false;
	}

private:
	void pushornil(std::vector<std::shared_ptr<node>> &v, int index, size_t target) {
		if (index < v.size()) {
			runexpressionhandler(v[index], target, 1);
		}
		else {
			emit(bytecode::instruction_opcode_CONSTANT, target, 2);
		}
	}

public:
	void runexpressionhandler(node &_expr, size_t target, size_t size) {
		auto found = expressionmap.find(typeid(_expr));

		if (found == expressionmap.end()) {
			std::cerr << "Unsupported expression when generating: " << gettypename(_expr) << std::endl;
			throw;
		}

		return found->second(*this, _expr, target, size);
	}

	void runexpressionhandler(std::shared_ptr<node> _expr, size_t target, size_t size) {
		return runexpressionhandler(*_expr.get(), target, size);
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

static void generate_numberexpression(bytecodegenerator &gen, node &expr, size_t target, size_t size) {
	if (size == 0) {
		return;
	}
	gen.emit(bytecode::instruction_opcode_NUMBER, target, gen.proto.numbers_size());
	gen.proto.add_numbers(dynamic_cast<expressions::numberexpression *>(&expr)->data);
}

static void generate_stringexpression(bytecodegenerator &gen, node &expr, size_t target, size_t size) {
	if (size == 0) {
		return;
	}
	gen.emit(bytecode::instruction_opcode_STRING, target, gen.proto.strings_size());
	gen.proto.add_strings(dynamic_cast<expressions::stringexpression *>(&expr)->data);
}

static void generate_enclosedexpression(bytecodegenerator &gen, node &expr, size_t target, size_t size) {
	auto &child = *dynamic_cast<expressions::enclosedexpression *>(&expr)->children[0].get();
	gen.runexpressionhandler(child, target, size > 0 ? 1 : 0);
}

std::unordered_map<string, bytecode::instruction_opcode> binoplookup = {
	{ "%", bytecode::instruction_opcode_MODULO },
	{ "-", bytecode::instruction_opcode_SUBTRACT },
	{ "+", bytecode::instruction_opcode_ADD },
	{ "..", bytecode::instruction_opcode_CONCAT },
	{ "^", bytecode::instruction_opcode_POWER },
	{ "and", bytecode::instruction_opcode_AND },
	{ "&&", bytecode::instruction_opcode_AND },
	{ "or", bytecode::instruction_opcode_OR },
	{ "||", bytecode::instruction_opcode_OR },
	{ "<", bytecode::instruction_opcode_LESSTHAN },
	{ "<=", bytecode::instruction_opcode_LESSTHANEQUAL },
	{ ">", bytecode::instruction_opcode_GREATERTHAN },
	{ ">=", bytecode::instruction_opcode_GREATERTHANEQUAL },
	{ "==", bytecode::instruction_opcode_EQUALS },
	{ "~=", bytecode::instruction_opcode_NOTEQUALS },
	{ "!=", bytecode::instruction_opcode_NOTEQUALS },
};

static void generate_binopexpression(bytecodegenerator &gen, node &_expr, size_t target, size_t size) {
	auto &expr = *dynamic_cast<expressions::binopexpression *>(&_expr);
	if (size == 0) {
		auto opcode = binoplookup[expr.op];
		if (opcode == bytecode::instruction_opcode_AND || opcode == bytecode::instruction_opcode_OR) {
			// these do not have overloads, can ignore binop part
			auto temp = gen.curfunc.gettemp();
			gen.runexpressionhandler(expr.lhs, temp, 1);
			gen.runexpressionhandler(expr.rhs, temp, 1);
			gen.curfunc.freetemp(temp);
		}
		else {
			auto temp = gen.curfunc.gettemp(2);
			gen.runexpressionhandler(expr.lhs, temp, 1);
			gen.runexpressionhandler(expr.rhs, temp + 1, 1);
			gen.emit(opcode, temp, temp, temp + 1);
			gen.curfunc.freetemp(temp, 2);
		}
	}
	else {
		gen.runexpressionhandler(expr.lhs, target, 1);
		auto temp = gen.curfunc.gettemp();
		gen.runexpressionhandler(expr.rhs, temp, 1);
		gen.emit(binoplookup[expr.op], target, target, temp);
		gen.curfunc.freetemp(temp);
	}
}

std::unordered_map<string, bytecode::instruction_opcode> unoplookup = {
	{ "not", bytecode::instruction_opcode_NOT },
	{ "!", bytecode::instruction_opcode_NOT },
	{ "#", bytecode::instruction_opcode_LENGTH },
	{ "-", bytecode::instruction_opcode_MINUS },
};

static void generate_unopexpression(bytecodegenerator &gen, node &_expr, size_t target, size_t size) {
	auto &expr = *dynamic_cast<expressions::unopexpression *>(&_expr);

	if (size > 0) {
		gen.runexpressionhandler(expr.expr, target, 1);
		gen.emit(unoplookup[expr.op], target, target);
	}
	else if (unoplookup[expr.op] != bytecode::instruction_opcode_NOT) {
		auto temp = gen.curfunc.gettemp(1);

		gen.runexpressionhandler(expr.expr, temp, 1);
		gen.emit(unoplookup[expr.op], temp, temp);

		gen.curfunc.freetemp(temp, 1);
	}
	else {
		gen.runexpressionhandler(expr.expr, target, 0);
	}
}

static void generate_nilexpression(bytecodegenerator &gen, node &expr, size_t target, size_t size) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 2);
}

static void generate_falseexpression(bytecodegenerator &gen, node &expr, size_t target, size_t size) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 1);
}

static void generate_trueexpression(bytecodegenerator &gen, node &expr, size_t target, size_t size) {
	if (size == 0) {
		return;
	}

	gen.emit(bytecode::instruction_opcode_CONSTANT, target, 0);
}

static void generate_indexexpression(bytecodegenerator &gen, node &_expr, size_t target, size_t size) {
	auto &expr = *dynamic_cast<expressions::indexexpression *>(&_expr);

	bool is_temp = size == 0;

	if (is_temp) {
		target = gen.curfunc.gettemp(1);
		size = 1;
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.curfunc.gettemp();
	gen.runexpressionhandler(expr.index, temp, 1);
	gen.emit(bytecode::instruction_opcode_INDEX, target, target, temp);
	gen.curfunc.freetemp(temp);

	if (is_temp) {
		gen.curfunc.freetemp(target, size);
	}
}

static void generate_dotexpression(bytecodegenerator &gen, node &_expr, size_t target, size_t size) {
	auto &expr = *dynamic_cast<expressions::dotexpression *>(&_expr);
	bool is_temp = size == 0;

	if (is_temp) {
		target = gen.curfunc.gettemp(1);
		size = 1;
	}
	gen.runexpressionhandler(expr.prefix, target, 1);

	auto temp = gen.curfunc.gettemp();
	expressions::stringexpression name(dynamic_cast<expressions::nameexpression *>(expr.index.get())->name);
	gen.runexpressionhandler(name, temp, 1);
	gen.emit(bytecode::instruction_opcode_INDEX, target, target, temp);
	gen.curfunc.freetemp(temp);

	if (is_temp) {
		gen.curfunc.freetemp(target, size);
	}
}

static void generate_nameexpression(bytecodegenerator &gen, node &_expr, size_t target, size_t size) {
	if (size == 0) {
		return;
	}

	auto &expr = *dynamic_cast<expressions::nameexpression *>(&_expr);
	if (gen.curfunc.haslocal(expr.name)) {
		gen.emit(bytecode::instruction_opcode_MOV, target, gen.curfunc.findlocal(expr.name), 0);
	}
	else if(gen.curfunc.hasupvalue(expr.name)) {
		// TODO
	}
	else {
		gen.emit(bytecode::instruction_opcode_ENVIRONMENT, target);
		expressions::stringexpression name(dynamic_cast<expressions::nameexpression *>(&expr)->name);
		auto temp = gen.curfunc.gettemp();
		gen.runexpressionhandler(name, temp, 1);
		gen.emit(bytecode::instruction_opcode_INDEX, target, target, temp);
		gen.curfunc.freetemp(temp);
	}
}

static void generate_functioncallexpression(bytecodegenerator &gen, node &_expr, size_t target, size_t size) {
	auto &expr = *dynamic_cast<expressions::functioncallexpression *>(&_expr);
	auto &arglist = *dynamic_cast<args *>(expr.arglist.get());

	size_t stacksize = std::max(size, arglist.children.size() + 1 + (expr.methodname ? 1 : 0));

	bool using_temp = stacksize > size;
	auto functionindex = target;
	auto argsindex = functionindex + 1;
	if (using_temp) {
		functionindex = gen.curfunc.gettemp(stacksize);
	}

	if (expr.methodname) {
		gen.runexpressionhandler(expr.funcexpr, argsindex, 1);
		expressions::stringexpression name(dynamic_cast<expressions::nameexpression *>(expr.methodname.get())->name);
		gen.runexpressionhandler(name, functionindex, 1);
		gen.emit(bytecode::instruction_opcode_INDEX, functionindex, argsindex, functionindex);

		argsindex++;
	}
	else {
		gen.runexpressionhandler(expr.funcexpr, functionindex, 1);
	}

	for (int i = 0; i < arglist.children.size(); i++) {
		// TODO: how vararg
		gen.runexpressionhandler(arglist.children[i], argsindex + i, 1);
	}

	gen.emit(bytecode::instruction_opcode_CALL, functionindex, arglist.children.size(), size);

	if (using_temp) {
		if (size != 0) {
			gen.emit(bytecode::instruction_opcode_MOV, target, functionindex, size);
		}
		gen.curfunc.freetemp(functionindex, stacksize);
	}
}


#define ADD(x) { typeid(expressions::x), generate_##x }
std::unordered_map<std::type_index, expressiongenerator> expressionmap = {
	ADD(numberexpression),
	ADD(nilexpression),
	ADD(falseexpression),
	ADD(trueexpression),
	ADD(stringexpression),
	ADD(enclosedexpression),
	ADD(binopexpression),
	ADD(unopexpression),
	// ADD(functioncallexpression),
	ADD(indexexpression),
	ADD(dotexpression),
	ADD(nameexpression),
	ADD(functioncallexpression),
};
#undef ADD


bytecode::prototype lorelai::vm::parse(chunk &data) {
	bytecodegenerator generator;
	std::shared_ptr<node> container = std::make_shared<chunk>(data);

	data.accept(generator, container);

	return generator.proto;
}

#ifndef BYTECODE_GENERATOR_HPP_
#define BYTECODE_GENERATOR_HPP_

#include "scope.hpp"
#include "statements.hpp"
#include "expressions.hpp"
#include "function.hpp"
#include "bytecode.hpp"
#ifdef __linux__
#include <cxxabi.h>

template <typename T>
static std::string gettypename(T &data) {
    auto ptr = std::unique_ptr<char, decltype(& std::free)>{
        abi::__cxa_demangle(typeid(data).name(), nullptr, nullptr, nullptr),
        std::free
    };
    return {ptr.get()};
}
#else
template <typename T>
static std::string gettypename(T &data) {
	return typeid(data).name();
}
#endif

namespace lorelai {
	namespace bytecode {
		class instruction;

		class bytecodegenerator : public variablevisitor {
			struct _ifqueue {
				bytecode::instruction *patch = nullptr;
				std::vector<bytecode::instruction *> jmpends;
				std::uint32_t target;
			};

			std::vector<_ifqueue> ifqueue;

			struct _assignmentqueue {
				std::uint32_t index;
				std::uint32_t size;
			};

			struct _loopqueue {
				int startinstr;
				std::uint32_t stackreserved;
				std::uint32_t extrastack;
				std::vector<bytecode::instruction *> patches;
			};

			std::vector<_loopqueue> loopqueue;

		public:
			using variablevisitor::visit;
			using variablevisitor::postvisit;

			bytecodegenerator(std::unordered_map<parser::node *, std::uint32_t> _protomap, variablevisitor &finder) : protomap(_protomap), funcptr(new function()), variablefinder(&finder) { }
			~bytecodegenerator() { delete funcptr; }

			LORELAI_POSTVISIT_FUNCTION(statements::localassignmentstatement);
			LORELAI_VISIT_FUNCTION(statements::assignmentstatement);

			LORELAI_VISIT_FUNCTION(statements::functioncallstatement) {
				runexpressionhandler(obj.callexpr, 0, 0);
				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::ifstatement);
			LORELAI_VISIT_FUNCTION(statements::elseifstatement);
			LORELAI_VISIT_FUNCTION(statements::elsestatement);
			LORELAI_POSTVISIT_FUNCTION(statements::ifstatement);

			LORELAI_VISIT_FUNCTION(statements::whilestatement);
			LORELAI_POSTVISIT_FUNCTION(statements::whilestatement);


			LORELAI_VISIT_FUNCTION(statements::repeatstatement) {
				_loopqueue data;
				data.startinstr = funcptr->proto->instructions_size();

				loopqueue.push_back(data);
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::repeatstatement);

			LORELAI_VISIT_FUNCTION(statements::breakstatement);

			LORELAI_VISIT_FUNCTION(statements::forinstatement);
			LORELAI_POSTVISIT_FUNCTION(statements::forinstatement);

			LORELAI_VISIT_FUNCTION(statements::fornumstatement);
			LORELAI_POSTVISIT_FUNCTION(statements::fornumstatement);

			LORELAI_VISIT_FUNCTION(statements::localfunctionstatement) {
				variablevisitor::visit(obj, container);
				pushfunc();

				return false;
			}

			LORELAI_POSTVISIT_FUNCTION(statements::localfunctionstatement) {
				variablevisitor::postvisit(obj, container);
				popfunc();

				emit(bytecode::instruction_opcode_FNEW, funcptr->varlookup[obj.name], protomap[&obj]);
			}

			LORELAI_VISIT_FUNCTION(statements::returnstatement);

			void onnewvariable(const variable &var) override {
				if (findfullvar(var)->writes > 0) {
					funcptr->newstackvariable(var.name);
				}
			}

			void onnewvariables(const std::vector<variable> &list) override {
				std::vector<string> names;
				for (auto &child : list) {
					if (findfullvar(child)->writes > 0) {
						names.push_back(child.name);
					}
				}
				funcptr->newstackvariables(names);
			}
			void onfreevariable(const variable &var) override {
				if (findfullvar(var)->writes > 0) {
					funcptr->freestackvariable(var.name);
				}
			}

			variable *findfullvar(const variable &v) {
				return variablefinder->scopes[v.scopeid]->find(v);
			}

			parser::node *findconstant(parser::node *_expr, string name) {
				auto v = scopes[protomap[_expr]]->find(name);
				if (!v) {
					return nullptr;
				}

				auto found = constantmap.find(*v);

				if (found != constantmap.end()) {
					return found->second;
				}

				return nullptr;
			}

		private:
			void pushornil(std::vector<lorelai::parser::node *> &v, int index, std::uint32_t target);

		public:
			void runexpressionhandler(lorelai::parser::node *_expr, std::uint32_t target, std::uint32_t size) {
				auto found = expressionmap.find(typeid(*_expr));

				if (found == expressionmap.end()) {
					throw exception(string("Unsupported expression when generating: ") + gettypename(_expr));
				}

				return found->second(*this, _expr, target, size);
			}

		public:
			instruction *emit(instruction_opcode opcode, std::uint32_t a = 0, std::uint32_t b = 0, std::uint32_t c = 0);

			void mov(std::uint32_t to, std::uint32_t from, std::uint32_t size = 1) {
				if (funcptr->proto->instructions_size() > 0) {
					auto &last = *funcptr->proto->mutable_instructions(funcptr->proto->instructions_size() - 1);
					if (last.op() == instruction_opcode_MOV && to == last.a() + last.c() + 1 && from == last.b() + last.c() + 1) {
						last.set_c(last.c() + size);
						return;
					}
				}

				emit(instruction_opcode_MOV, to, from, size - 1);
			}

		public:
			int add(string str) {
				return funcptr->add(str);
			}

			int add(number num) {
				return funcptr->add(num);
			}

		private:
			void pushfunc() {
				function fn(funcptr);
				funcptr->children.push_back(fn);
				funcptr = &funcptr->children.back();
			}

			void popfunc() {
				funcptr = funcptr->parent;
			}

		public:
			function *funcptr = nullptr;

			std::unordered_map<parser::node *, std::uint32_t> protomap;
			variablevisitor *variablefinder = nullptr;
			std::unordered_map<variable, parser::node *> constantmap;
		};
	}
}

#endif // BYTECODE_GENERATOR_HPP_
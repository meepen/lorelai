#ifndef BYTECODE_GENERATOR_HPP_
#define BYTECODE_GENERATOR_HPP_

#include "scope.hpp"
#include "statements.hpp"
#include "bcexpressions.hpp"
#include "node.hpp"
#include "function.hpp"
#include "bytecode.hpp"
#include "prototype.hpp"
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
				optional<prototype::instruct_ptr> patch;
				std::vector<prototype::instruct_ptr> jmpends;
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
				std::vector<prototype::instruct_ptr> patches;
			};

			std::vector<_loopqueue> loopqueue;

		public:
			using variablevisitor::visit;
			using variablevisitor::postvisit;

			bytecodegenerator() : funcptr(new function()) { }
			~bytecodegenerator() {
				for (auto &child : allocatedconsts) {
					delete child;
				}

				delete funcptr;
			}

			LORELAI_POSTVISIT_FUNCTION(statements::localassignmentstatement);
			LORELAI_VISIT_FUNCTION(statements::assignmentstatement);

			LORELAI_VISIT_FUNCTION(statements::functioncallstatement) {
				runexpressionhandler(*obj.callexpr, 0, 0);
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
				data.startinstr = funcptr->proto.instructions_size();

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

				return false;
			}

			LORELAI_POSTVISIT_FUNCTION(statements::localfunctionstatement) {
				auto id = funcptr->protoid;
				variablevisitor::postvisit(obj, container);

				emit(prototype::OP_FNEW, funcptr->varlookup[obj.name], id);
			}

			LORELAI_VISIT_FUNCTION(statements::returnstatement);


		public: // OVERRIDE
			void onnewvariable(const variable &var) override {
				if (var.argnum) {
					funcptr->newargument(var.name, var.argnum.value());
				}
				else {
					funcptr->newstackvariable(var.name);
				}
			}

			void onnewvariables(const std::vector<variable> &list) override {
				std::vector<string> names;
				for (auto &child : list) {
					names.push_back(child.name);
				}
				funcptr->newstackvariables(names);
			}

			void onfreevariable(const variable &var) override {
				funcptr->freestackvariable(var.name);
			}

			void onnewscope(bool isfunction) override {
				variablevisitor::onnewscope(isfunction);
				if (isfunction) {
					pushfunc();
				}
			}

			void onfreescope(bool isfunction) override {
				variablevisitor::onfreescope(isfunction);
				if (isfunction) {
					popfunc();
				}
			}

		private:
			void pushornil(std::vector<lorelai::parser::node *> &v, int index, std::uint32_t target);

		public:
			void runexpressionhandler(const lorelai::parser::node &_expr, std::uint32_t target, std::uint32_t size) {
				auto &expr = trycollapse(_expr);
				auto found = expressionmap.find(typeid(expr));

				if (found == expressionmap.end()) {
					throw exception(string("Unsupported expression when generating: ") + gettypename(expr));
				}

				return found->second(*this, expr, target, size);
			}

			void runexpressionhandler(const lorelai::parser::node *_expr, std::uint32_t target, std::uint32_t size) {
				return runexpressionhandler(*_expr, target, size);
			}

		public:
			prototype::instruct_ptr emit(prototype::_opcode opcode, std::uint8_t a = 0, std::uint8_t b = 0, std::uint8_t c = 0);

			void mov(std::uint32_t to, std::uint32_t from, std::uint32_t size = 1) {
				if (size == 1) {
					emit(prototype::OP_MOV1, to, from, size - 1);
				}
				else {
					emit(prototype::OP_MOV, to, from, size - 1);
				}
			}

			void setstackvar(std::uint32_t target, const string &name, const parser::node &obj) {
				if (funcptr->hasvariable(name)) {
					runexpressionhandler(obj, target, 1);
				}
			}

			const parser::node &trycollapse(const parser::node &);

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

			std::vector<const parser::node *> allocatedconsts;
			std::unordered_map<variable, const parser::node *> initmap;
		};
	}
}

#endif // BYTECODE_GENERATOR_HPP_
#ifndef BYTECODE_GENERATOR_HPP_
#define BYTECODE_GENERATOR_HPP_

#include "visitor.hpp"
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

		class bytecodegenerator : public parser::visitor {
			struct _ifqueue {
				bytecode::instruction *patch = nullptr;
				std::vector<bytecode::instruction *> jmpends;
				size_t target;
			};

			std::vector<_ifqueue> ifqueue;

			struct _loopqueue {
				int startinstr;
				size_t stackreserved;
				size_t extrastack;
				std::vector<bytecode::instruction *> patches;
			};

			std::vector<_loopqueue> loopqueue;

		public:
			using visitor::visit;
			using visitor::postvisit;

			LORELAI_VISIT_FUNCTION(statements::localassignmentstatement);
			LORELAI_VISIT_FUNCTION(statements::assignmentstatement);

			LORELAI_VISIT_FUNCTION(statements::functioncallstatement) {
				runexpressionhandler(obj.children[0], 0, 0);
				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::ifstatement);
			LORELAI_VISIT_FUNCTION(statements::elseifstatement);
			LORELAI_VISIT_FUNCTION(statements::elsestatement);
			LORELAI_POSTVISIT_FUNCTION(statements::ifstatement);

			LORELAI_VISIT_FUNCTION(statements::dostatement) {
				curfunc.pushscope();
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::dostatement) {
				curfunc.popscope();
				return false;
			}

			LORELAI_VISIT_FUNCTION(statements::whilestatement);
			LORELAI_POSTVISIT_FUNCTION(statements::whilestatement);


			LORELAI_VISIT_FUNCTION(statements::repeatstatement) {
				_loopqueue data;
				data.startinstr = curfunc.proto->instructions_size();

				loopqueue.push_back(data);
				return false;
			}
			LORELAI_POSTVISIT_FUNCTION(statements::repeatstatement);

			LORELAI_VISIT_FUNCTION(statements::breakstatement);

			LORELAI_VISIT_FUNCTION(statements::forinstatement);
			LORELAI_POSTVISIT_FUNCTION(statements::forinstatement);

			LORELAI_VISIT_FUNCTION(statements::fornumstatement);
			LORELAI_POSTVISIT_FUNCTION(statements::fornumstatement);

			LORELAI_VISIT_FUNCTION(statements::returnstatement);

		private:
			void pushornil(std::vector<std::shared_ptr<lorelai::parser::node>> &v, int index, size_t target);

		public:
			void runexpressionhandler(lorelai::parser::node &_expr, size_t target, size_t size) {
				auto found = expressionmap.find(typeid(_expr));

				if (found == expressionmap.end()) {
					throw exception(string("Unsupported expression when generating: ") + gettypename(_expr));
				}

				return found->second(*this, _expr, target, size);
			}

			void runexpressionhandler(std::shared_ptr<lorelai::parser::node> _expr, size_t target, size_t size) {
				return runexpressionhandler(*_expr.get(), target, size);
			}

		public:
			instruction *emit(instruction_opcode opcode);
			instruction *emit(instruction_opcode opcode, std::uint32_t a);
			instruction *emit(instruction_opcode opcode, std::uint32_t a, std::uint32_t b);
			instruction *emit(instruction_opcode opcode, std::uint32_t a, std::uint32_t b, std::uint32_t c);

		public:
			int add(string str) {
				return curfunc.add(str);
			}

			int add(number num) {
				return curfunc.add(num);
			}

		public:
			function curfunc;
		};
	}
}

#endif // BYTECODE_GENERATOR_HPP_
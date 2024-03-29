#include "state.hpp"
#include "bytecode.hpp"

using namespace lorelai;
using namespace lorelai::vm;

void state::loadfunction(const std::string &code) {
	loadfunction(bytecode::create(code));
}
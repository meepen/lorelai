#include "x86state.hpp"

using namespace lorelai;
using namespace lorelai::vm;

#ifdef LORELAI_X86_FASTEST

std::shared_ptr<state> state::create() {
	return std::make_shared<x86state>();
}

#endif // LORELAI_X86_FASTEST

void x86state::loadstring(const std::string &code) {
	throw std::exception();
}
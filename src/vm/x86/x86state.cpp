#include "x86state.hpp"

using namespace lorelai;
using namespace lorelai::vm;

state *state::create() {
	return new x86state();
}
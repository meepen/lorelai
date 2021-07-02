#include "../object.hpp"
#include "../software.hpp"
#include <string>

using namespace lorelai;
using namespace lorelai::vm;

state::_retdata cfunctionobject::call(softwarestate &state, int nrets, int nargs) {
	auto retsize = data(state, nrets, nargs);
	return {
		state->base + state->top - retsize,
		retsize
	};
}

object cfunctionobject::create(softwarestate &state, luafunction func) {
	return *state.memory.allocate<cfunctionobject>(CFUNCTION, func)->get<cfunctionobject>();
}
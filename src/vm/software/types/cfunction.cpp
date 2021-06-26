#include "../object.hpp"
#include "../software.hpp"
#include <exception>
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
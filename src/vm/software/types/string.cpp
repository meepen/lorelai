#include "../software.hpp"

using namespace lorelai;
using namespace lorelai::vm;

object stringobject::create(softwarestate &state, string str) {
	auto found = state.stringmap.find(str);
	if (found != state.stringmap.end()) {
		return *found->second;
	}

	auto ret = state.memory.allocate<stringobject>(STRING, str)->get<stringobject>();

	state.stringmap[str] = ret;

	return *ret;
}

object stringobject::metatable(softwarestate &state) const {
	return state.string_metatable;
}
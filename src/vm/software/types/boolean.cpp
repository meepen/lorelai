#include "../object.hpp"

using namespace lorelai;
using namespace lorelai::vm;

objectcontainer boolobject::create(softwarestate &state, bool b) {
	return b ? state.trueobj : state.falseobj;
}
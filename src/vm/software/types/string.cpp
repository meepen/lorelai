#include "../object.hpp"
#include "../software.hpp"

using namespace lorelai;
using namespace lorelai::vm;

objectcontainer stringobject::create(softwarestate &state, string str) {
	return state.stringallocator.take(str);
}
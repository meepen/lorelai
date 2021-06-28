#include "../object.hpp"
#include "../software.hpp"

using namespace lorelai;
using namespace lorelai::vm;


object tableobject::create(softwarestate &state) {
	auto r = object(state.tableallocator.take(), true);
	// TODO(possibly?) clear table
	return r;
}
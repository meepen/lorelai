#include "../object.hpp"

using namespace lorelai;
using namespace lorelai::vm;


objectcontainer tableobject::create(softwarestate &state) {
	return state.tableallocator.take();
}
#include "../object.hpp"

using namespace lorelai;
using namespace lorelai::vm;

objectcontainer numberobject::create(softwarestate &state, number num) {
	return state.numberallocator.take(num);
}
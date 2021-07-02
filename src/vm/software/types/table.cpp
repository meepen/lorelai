#include "../object.hpp"
#include "../software.hpp"

using namespace lorelai;
using namespace lorelai::vm;

object tableobject::create(softwarestate &state) {
	return *state.memory.allocate<tableobject>(TABLE)->get<tableobject>();
}
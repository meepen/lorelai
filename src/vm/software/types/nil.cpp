#include "../object.hpp"

using namespace lorelai;
using namespace lorelai::vm;

objectcontainer nilobject::create(softwarestate &state) {
	return state.nil;
}
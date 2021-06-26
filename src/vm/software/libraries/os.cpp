#include "../libraries.hpp"
#include "../software.hpp"
#include "../object.hpp"

using namespace lorelai;
using namespace lorelai::vm;

static int os_clock(softwarestate &state, int nrets, int nargs) {
	state.loadnumber(static_cast<number>(clock()) / CLOCKS_PER_SEC);

	return 1;
}

library vm::os[] = {
	{ "clock", os_clock },
	{ nullptr, nullptr }
};
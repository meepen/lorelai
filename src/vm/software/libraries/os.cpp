#include "../libraries.hpp"
#include "../software.hpp"
#include "../object.hpp"

using namespace lorelai;
using namespace lorelai::vm;

static int os_clock(softwarestate &state, int nargs) {
	state.loadnumber(static_cast<number>(clock()) / CLOCKS_PER_SEC);

	return 1;
}

static int os_exit(softwarestate &state, int nargs) {
	int code = 0;
	if (nargs >= 1) {
		code = state[1].tonumber(state);
	}

	exit(code);
	return 0;
}

library vm::os[] = {
	{ "clock", os_clock },
	{ "exit", os_exit },
	{ nullptr, nullptr }
};
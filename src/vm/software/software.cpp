#include "bytecode.hpp"
#include "software.hpp"
#include "object.hpp"
#include <iostream>

using namespace lorelai;
using namespace lorelai::vm;

std::shared_ptr<state> softwarestate::create() {
	return std::make_shared<softwarestate>();
}

std::shared_ptr<state> state::create() {
	return softwarestate::create();
}


#ifdef LORELAI_SOFTWARE_FASTEST

std::shared_ptr<state> state::createfastest() {
	return softwarestate::create();
}

#endif // LORELAI_X86_FASTEST

size_t print(softwarestate &state, objectcontainer *out, size_t nrets, size_t nargs) {
	for (size_t i = 1; i <= nargs; i++) {
		std::cout << out[i]->type();
	}
	std::cout << std::endl;

	return 0;
}

void softwarestate::initlibs() {
	registry->rawset(*this, std::make_shared<stringobject>("print"), std::make_shared<cfunctionobject>(print));
}
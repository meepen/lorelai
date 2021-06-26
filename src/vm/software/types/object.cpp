#include "../object.hpp"
#include <cmath>

using namespace lorelai;
using namespace lorelai::vm;

void object::concat(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<stringobject>(tostring(state) + rhs->tostring(state));
}

void object::add(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(tonumber(state) + rhs->tonumber(state));
}

void object::subtract(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(tonumber(state) - rhs->tonumber(state));
}
void object::divide(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(tonumber(state) / rhs->tonumber(state));
}

void object::multiply(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(tonumber(state) * rhs->tonumber(state));
}

void object::power(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(std::pow(tonumber(state), rhs->tonumber(state)));
}

void object::modulo(softwarestate &state, objectcontainer &out, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	auto a = tonumber(state);
	auto b = rhs->tonumber(state);

	out = std::make_shared<numberobject>(a - b * std::floor(a / b));
}

bool object::lessthan(softwarestate &state, objectcontainer rhs) {
	if (metatable() && metatable() == rhs->metatable()) {
		// todo
	}

	return tonumber(state) < rhs->tonumber(state);
}

bool object::greaterthan (softwarestate &state, objectcontainer rhs) {

	return tonumber(state) > rhs->tonumber(state);
}
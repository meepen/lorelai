#include "../object.hpp"
#include <cmath>

using namespace lorelai;
using namespace lorelai::vm;

void object::add(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs) {
	if (lhs->metatable() && lhs->metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(lhs->tonumber(state) + rhs->tonumber(state));
}

void object::subtract(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs) {
	if (lhs->metatable() && lhs->metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(lhs->tonumber(state) - rhs->tonumber(state));
}
void object::divide(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs) {
	if (lhs->metatable() && lhs->metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(lhs->tonumber(state) / rhs->tonumber(state));
}

void object::multiply(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs) {
	if (lhs->metatable() && lhs->metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(lhs->tonumber(state) * rhs->tonumber(state));
}

void object::power(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs) {
	if (lhs->metatable() && lhs->metatable() == rhs->metatable()) {
		// todo
	}

	out = std::make_shared<numberobject>(std::pow(lhs->tonumber(state), rhs->tonumber(state)));
}

void object::modulo(softwarestate &state, objectcontainer &out, objectcontainer lhs, objectcontainer rhs) {
	if (lhs->metatable() && lhs->metatable() == rhs->metatable()) {
		// todo
	}

	auto a = lhs->tonumber(state);
	auto b = rhs->tonumber(state);

	out = std::make_shared<numberobject>(a - b * std::floor(a / b));
}
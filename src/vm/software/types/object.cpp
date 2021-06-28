#include "../object.hpp"
#include "../software.hpp"
#include <cmath>
#include <sstream>

using namespace lorelai;
using namespace lorelai::vm;

bool referenceobject::equals(const object &other) const {
	return this == other.raw.ref.get();
}

bool referenceobject::equals(softwarestate &state, object &rhs) {
	return rhs.type == _typeid() && this == rhs.raw.ref.get();
}

void referenceobject::concat(softwarestate &state, object &out, object &rhs) {
	out.set(tostring(state) + rhs.tostring(state));
}

bool object::equals(softwarestate &state, object &other) {
	// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
	if (type >= TABLE) {
		return raw.ref->equals(state, other);
	}
	else if (other.type >= TABLE) {
		return other.raw.ref->equals(state, *this);
	}

	// otherwise, check types
	// regular object can only be equal to the same type
	if (type != other.type) {
		return false;
	}

	switch (type) {
	case NUMBER:
		return raw.num == other.raw.num;
	case STRING:
		return raw.str == other.raw.str;
	case BOOL:
		return raw.b == other.raw.b;
	case NIL:
		return true;
	default:
		return false;
	}
}

void object::add(softwarestate &state, object &out, object &other) {
	// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
	if (type >= TABLE) {
		return raw.ref->add(state, out, other);
	}
	else if (other.type >= TABLE) {
		return other.raw.ref->add(state, out, *this);
	}

	// otherwise, check types
	// can only add number to number
	if (type != other.type || type != NUMBER) {
		throw exception(string("cannot add ") + gettypename() + " and " + other.gettypename());
	}
	
	out.set(raw.num + other.raw.num);
}

void object::subtract(softwarestate &state, object &out, object &other) {
	// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
	if (type >= TABLE) {
		return raw.ref->add(state, out, other);
	}
	else if (other.type >= TABLE) {
		return other.raw.ref->add(state, out, *this);
	}

	// otherwise, check types
	// can only add number to number
	if (type != other.type || type != NUMBER) {
		throw exception(string("cannot subtract ") + gettypename() + " and " + other.gettypename());
	}
	
	out.set(raw.num - other.raw.num);
}
void object::divide(softwarestate &state, object &out, object &other) {
	// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
	if (type >= TABLE) {
		return raw.ref->add(state, out, other);
	}
	else if (other.type >= TABLE) {
		return other.raw.ref->add(state, out, *this);
	}

	// otherwise, check types
	// can only add number to number
	if (type != other.type || type != NUMBER) {
		throw exception(string("cannot divide ") + gettypename() + " and " + other.gettypename());
	}
	
	out.set(raw.num / other.raw.num);
}

void object::multiply(softwarestate &state, object &out, object &other) {
	// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
	if (type >= TABLE) {
		return raw.ref->add(state, out, other);
	}
	else if (other.type >= TABLE) {
		return other.raw.ref->add(state, out, *this);
	}

	// otherwise, check types
	// can only add number to number
	if (type != other.type || type != NUMBER) {
		throw exception(string("cannot multiply ") + gettypename() + " and " + other.gettypename());
	}
	
	out.set(raw.num * other.raw.num);
}

void object::power(softwarestate &state, object &out, object &other) {
	// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
	if (type >= TABLE) {
		return raw.ref->add(state, out, other);
	}
	else if (other.type >= TABLE) {
		return other.raw.ref->add(state, out, *this);
	}

	// otherwise, check types
	// can only add number to number
	if (type != other.type || type != NUMBER) {
		throw exception(string("cannot power ") + gettypename() + " and " + other.gettypename());
	}
	
	out.set(std::pow(raw.num, other.raw.num));
}

void object::modulo(softwarestate &state, object &out, object &other) {
	// if either can potentially have a custom metamethod be ran, pass through to the reference object to check
	if (type >= TABLE) {
		return raw.ref->add(state, out, other);
	}
	else if (other.type >= TABLE) {
		return other.raw.ref->add(state, out, *this);
	}

	// otherwise, check types
	// can only add number to number
	if (type != other.type || type != NUMBER) {
		throw exception(string("cannot subtract ") + gettypename() + " and " + other.gettypename());
	}
	
	out.set(raw.num - other.raw.num * std::floor(raw.num / other.raw.num));
}

bool object::lessthan(softwarestate &state, object &other) {
	return tonumber(state) < other.tonumber(state);
}

bool object::greaterthan(softwarestate &state, object &other) {
	return tonumber(state) > other.tonumber(state);
}

void object::concat(softwarestate &state, object &out, object &other) {
	out.set(tostring(state) + other.tostring(state));
}

bool object::tobool(softwarestate &state) {
	switch (type) {
		case NIL:
			return false;
		case BOOL:
			return raw.b;
		default:
			return true;
	}
}

number object::tonumber(softwarestate &state) {
	if (type == NUMBER) {
		return raw.num;
	}
	else if (type == STRING) {
		return lorelai::tonumber(raw.str);
	}
	
	throw exception(string("cannot convert ") + gettypename() + " to number");
}

string object::tostring(softwarestate &state) {
	switch (type) {
	case NIL:
		return "nil";
	case BOOL:
		return raw.b ? "true" : "false";
	case NUMBER:
		{
			std::ostringstream stream;
			stream.precision(13);
			stream << raw.num;
			return stream.str();
		}
	case STRING:
		return raw.str;

	default:
		throw exception(string("cannot convert ") + gettypename() + " to string");
	}
}
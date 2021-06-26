#include "../object.hpp"
#include <exception>
#include <string>

using namespace lorelai;
using namespace lorelai::vm;

size_t cfunctionobject::call(softwarestate &state, std::shared_ptr<object> *out, size_t nrets, size_t nargs) {
	return data(state, out, nrets, nargs);
}
#include "../object.hpp"
#include "../software.hpp"
#include <string>

using namespace lorelai;
using namespace lorelai::vm;

bool referenceobject::equals_rhs(softwarestate &state, object &rhs) {
    return this == &rhs.ref();
}
bool referenceobject::equals_lhs(softwarestate &state, object &rhs) {
    return this == &rhs.ref();
}
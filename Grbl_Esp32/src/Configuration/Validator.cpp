#include "Validator.h"

#include "Configurable.h"

#include <cstring>

namespace Configuration
{
    void Validator::handleDetail(const char* name, Configurable* value) {
        value->validate();
        value->handle(*this);
    }
}
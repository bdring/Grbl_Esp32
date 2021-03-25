#include "AfterParse.h"

#include "Configurable.h"

#include <cstring>

namespace Configuration
{
    void AfterParse::handleDetail(const char* name, Configurable* value) {
        value->afterParse();
        value->handle(*this);
    }
}

// This file loads custom code from the Custom/ subdirectory if
// CUSTOM_CODE_FILENAME is defined.

#include "grbl.h"

#ifdef CUSTOM_CODE_FILENAME
#    include CUSTOM_CODE_FILENAME
#endif

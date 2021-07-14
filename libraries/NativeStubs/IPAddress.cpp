#include <IPAddress.h>

String IPAddress::toString() const {
    char szRet[16];
    sprintf(szRet, "%u.%u.%u.%u", _address.bytes[0], _address.bytes[1], _address.bytes[2], _address.bytes[3]);
    return String(szRet);
}

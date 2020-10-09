#include "WString.h"

StringSumHelper& operator +(const StringSumHelper& lhs, const String& rhs) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(rhs);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, const char* cstr) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(cstr);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, char c) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(c);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, unsigned char num) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(num);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, int num) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(num);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, unsigned int num) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(num);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, long num) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(num);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, unsigned long num) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(num);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, float num) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(num);
    return a;
}

StringSumHelper& operator +(const StringSumHelper& lhs, double num) {
    StringSumHelper& a = const_cast<StringSumHelper&>(lhs);
    a.concat(num);
    return a;
}

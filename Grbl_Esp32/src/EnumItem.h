#pragma once

struct EnumItem {
    EnumItem() : value(0), name(nullptr) {}
    EnumItem(int val, const char* n) : value(val), name(n) {}

    int         value;
    const char* name;
};

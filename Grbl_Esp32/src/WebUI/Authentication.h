#pragma once

namespace WebUI {
    //Authentication level
    enum class auth_t : uint8_t { LEVEL_GUEST = 0, LEVEL_USER = 1, LEVEL_ADMIN = 2 };

    static const int MIN_LOCAL_PASSWORD_LENGTH = 1;
    static const int MAX_LOCAL_PASSWORD_LENGTH = 16;

    void remove_password(char* str, auth_t& auth_level);
}

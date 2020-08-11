#pragma once

//Authentication level
typedef enum { LEVEL_GUEST = 0, LEVEL_USER = 1, LEVEL_ADMIN = 2 } auth_t;

#define MIN_LOCAL_PASSWORD_LENGTH 1
#define MAX_LOCAL_PASSWORD_LENGTH 16

void remove_password(char* str, auth_t& auth_level);

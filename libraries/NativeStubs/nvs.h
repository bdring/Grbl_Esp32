#pragma once

const esp_err_t ESP_ERR_NVS_INVALID_HANDLE = 3;
const esp_err_t ESP_ERR_NVS_INVALID_NAME   = 2;
const esp_err_t ESP_ERR_NVS_INVALID_LENGTH = 1;

const int        NVS_READWRITE = 0;
inline esp_err_t nvs_open(const char* s, int mode, int* handle) {
    return 0;
}
inline esp_err_t nvs_get_i32(int _handle, const char* _keyName, int* value) {
    return -1;
}
inline esp_err_t nvs_set_i32(int _handle, const char* _keyName, int value) {
    return -1;
}
inline esp_err_t nvs_get_i8(int _handle, const char* _keyName, signed char* value) {
    return -1;
}
inline esp_err_t nvs_set_i8(int _handle, const char* _keyName, int value) {
    return -1;
}
inline esp_err_t nvs_get_str(int _handle, const char* _keyName, void* p, size_t* len) {
    return -1;
}
inline esp_err_t nvs_set_str(int _handle, const char* _keyName, const char* value) {
    return -1;
}
inline void      nvs_erase_key(int _handle, const char* key) {}
inline esp_err_t nvs_get_blob(int handle, const char* _keyName, void* currentValue, size_t* len) {
    return -1;
}
inline esp_err_t nvs_set_blob(int handle, const char* _keyName, void* currentValue, size_t len) {
    return -1;
}

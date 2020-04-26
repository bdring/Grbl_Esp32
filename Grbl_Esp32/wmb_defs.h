#pragma once

#include <stdint.h>
#define SPINDLE_TYPE_PWM "PWM"
#define DEFAULT_SPINDLE_BIT_PRECISION 10

// From "nvs.h" in SDK
typedef int esp_err_t;
inline esp_err_t nvs_get_i32(int h, const char *s, int32_t *p) {
    *p = 0;
    return 0;
}
inline esp_err_t nvs_get_i8(int h, const char *s, int8_t *p) {
    *p = 0;
    return 0;
}
inline esp_err_t nvs_get_str(int h, const char *s, char *p, size_t *len) {
    *len = 3;
    if (p) {
        p[0] = 'H'; p[1] = 'i'; p[2] = '\0';
    }
    return 0;
}
inline esp_err_t nvs_set_i8(int h, const char *s, int8_t v) {
    return 0;
}
inline esp_err_t nvs_set_i32(int h, const char *s, int32_t v) {
    return 0;
}
inline esp_err_t nvs_set_str(int h, const char *s, const char *v) {
    return 0;
}
inline esp_err_t nvs_erase_key(int h, const char *s) {
    return 0;
}


// These status values are just assigned at random, for testing
// they need to be synced with the rest of Grbl
typedef enum {
  STATUS_OK = 0,
  STATUS_INVALID_STATEMENT,
  STATUS_NUMBER_RANGE,
  STATUS_INVALID_VALUE,
  STATUS_IDLE_ERROR,
} err_t;

inline err_t check_homing_enable(const char *) { return STATUS_OK; }
inline err_t my_spindle_init(const char *)  { return STATUS_OK; }
inline err_t limits_init(const char *) { return STATUS_OK; }
inline err_t also_soft_limit(const char *)  { return STATUS_OK; }
void settings_restore(int);
void report_feedback_message(const char *);
err_t report_normal_settings(uint8_t);
err_t report_extended_settings(uint8_t);
err_t report_gcode_modes(uint8_t);
err_t report_build_info(uint8_t);
err_t report_startup_lines(uint8_t);
err_t toggle_check_mode(uint8_t);
err_t disable_alarm_lock(uint8_t);
err_t report_ngc(uint8_t);
err_t home_all(uint8_t);
err_t home_x(uint8_t);
err_t home_y(uint8_t);
err_t home_z(uint8_t);
err_t home_a(uint8_t);
err_t home_b(uint8_t);
err_t home_c(uint8_t);
err_t sleep(uint8_t);
err_t gc_execute_line(char *line, uint8_t client);

void mc_reset();

static inline err_t check_motor_settings(const char *) { return STATUS_OK; }
static inline err_t settings_spi_driver_init(const char *) { return STATUS_OK; }

#define MAX_N_AXIS 6

// From config.h
#define ESP_RADIO_OFF 0
#define ESP_WIFI_STA 1
#define ESP_WIFI_AP  2
#define ESP_BT       3

#define DEFAULT_RADIO_MODE ESP_WIFI_STA

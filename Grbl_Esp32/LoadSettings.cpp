// Transfer object-based setting to old-style settings
// Ultimately the code that uses the old settings
// should be changed to use the Settings objects directly

#include "grbl.h"
#ifdef NEW_SETTINGS

#include "SettingsDefinitions.h"

void setBit(bool flag, int32_t mask)
{
    if (flag) {
        settings.flags |= mask;
    }
}
void transfer_settings()
{
    // axes
    //    for (int i = 0; i < N_AXIS; i++) {
    for (int i = 0; i < MAX_N_AXIS; i++) {
        AxisSettings* so = axis_settings[i];
        settings.steps_per_mm[i] = so->steps_per_mm->get();
        settings.max_rate[i] = so->max_rate->get();
        settings.acceleration[i] = so->acceleration->get();
        settings.max_travel[i] = -so->max_travel->get(); // Stored as negative
        settings.current[i] = so->run_current->get();
        settings.hold_current[i] = so->hold_current->get();
        settings.microsteps[i] = so->microsteps->get();
        settings.stallguard[i] = so->stallguard->get();
    }

    // flags
    setBit(report_inches->get(), BITFLAG_REPORT_INCHES);
    setBit(laser_mode->get(), BITFLAG_LASER_MODE);
    setBit(step_enable_invert->get(), BITFLAG_INVERT_ST_ENABLE);
    setBit(hard_limits->get(), BITFLAG_HARD_LIMIT_ENABLE);
    setBit(homing_enable->get(), BITFLAG_HOMING_ENABLE);
    setBit(soft_limits->get(), BITFLAG_SOFT_LIMIT_ENABLE);
    setBit(limit_invert->get(), BITFLAG_INVERT_LIMIT_PINS);
    setBit(probe_invert->get(), BITFLAG_INVERT_PROBE_PIN);

    // Homing
    settings.homing_pulloff = homing_pulloff->get();
    settings.homing_seek_rate = homing_seek_rate->get();
    settings.homing_feed_rate = homing_feed_rate->get();
    settings.homing_dir_mask = homing_dir_mask->get();
    settings.homing_debounce_delay = homing_debounce->get();

    // Stepper motors
    settings.stepper_idle_lock_time = stepper_idle_lock_time->get();
    settings.step_invert_mask = step_invert_mask->get();
    settings.dir_invert_mask = dir_invert_mask->get();
    settings.pulse_microseconds = pulse_microseconds->get();

    // Motion
    settings.junction_deviation = junction_deviation->get();
    settings.arc_tolerance = arc_tolerance->get();

    // Spindle
    settings.rpm_min = rpm_min->get();
    settings.rpm_max = rpm_max->get();
//    settings.spindle_pwm_precision_bits = spindle_pwm_precision->get();
    settings.spindle_pwm_freq = spindle_pwm_freq->get();
    settings.spindle_pwm_min_value = spindle_pwm_min_value->get();
    settings.spindle_pwm_max_value = spindle_pwm_max_value->get();
    settings.spindle_pwm_off_value = spindle_pwm_off_value->get();
}

void settings_restore(uint8_t restore_flag) {
#if defined(ENABLE_BLUETOOTH) || defined(ENABLE_WIFI)
    if (restore_flag & SETTINGS_RESTORE_WIFI_SETTINGS) {
#ifdef ENABLE_WIFI
        wifi_config.reset_settings();
#endif
#ifdef ENABLE_BLUETOOTH
        bt_config.reset_settings();
#endif
    }
#endif
    if (restore_flag & SETTINGS_RESTORE_DEFAULTS) {
        for (Setting *s = SettingsList; s; s = s->next()) {
            bool restore_startup = restore_flag & SETTINGS_RESTORE_STARTUP_LINES;
            if (!s->getWebuiName()) {
                const char *name = s->getName();
                if (restore_startup || ((strcmp(name, "N0") != 0) && (strcmp(name, "N1") == 0))) {
                    s->setDefault();
                }
            }
        }
        transfer_settings();
        // TODO commit changes
    }
    if (restore_flag & SETTINGS_RESTORE_PARAMETERS) {
        uint8_t idx;
        float coord_data[N_AXIS];
        memset(&coord_data, 0, sizeof(coord_data));
        for (idx = 0; idx <= SETTING_INDEX_NCOORD; idx++)  settings_write_coord_data(idx, coord_data);
    }
    if (restore_flag & SETTINGS_RESTORE_BUILD_INFO) {
        EEPROM.write(EEPROM_ADDR_BUILD_INFO, 0);
        EEPROM.write(EEPROM_ADDR_BUILD_INFO + 1, 0); // Checksum
        EEPROM.commit();
    }
}

void load_settings()
{
    for (Setting *s = SettingsList; s; s = s->next()) {
        s->load();
    }
}

extern void list_settings();
extern void make_settings();
void settings_init()
{
    make_settings();
    load_settings();
    transfer_settings();
}
#endif

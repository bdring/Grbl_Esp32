#pragma once

// Timer stuff used by Stepper.cpp
typedef struct {
    int  divider;
    int  counter_dir;
    int  counter_en;
    int  alarm_en;
    int  intr_type;
    bool auto_reload;
} timer_config_t;

const int           TIMER_COUNT_UP   = 0;
const int           TIMER_PAUSE      = 0;
const int           TIMER_ALARM_EN   = 0;
const int           TIMER_INTR_LEVEL = 0;
typedef int         timer_group_t;
typedef int         timer_idx_t;
const timer_group_t TIMER_GROUP_0 = 0;
const timer_idx_t   TIMER_0       = 0;

inline void timer_set_counter_value(timer_group_t group, timer_idx_t idx, uint64_t ticks) {}
inline void timer_set_alarm_value(timer_group_t group, timer_idx_t idx, uint64_t ticks) {}
inline void timer_init(timer_group_t group, timer_idx_t idx, timer_config_t* conf) {}
inline void timer_enable_intr(timer_group_t group, timer_idx_t idx) {}
inline void timer_start(timer_group_t group, timer_idx_t idx) {}
inline void timer_pause(timer_group_t group, timer_idx_t idx) {}
inline void timer_isr_register(timer_group_t group, timer_idx_t idx, void (*handler)(void*), void* arg, int a, void* arg1) {}

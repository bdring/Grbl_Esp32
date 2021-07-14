#include "Grbl.h"
#include "Motors/Motor.h"
#include "Motors/TrinamicDriver.h"
#include "Motors/TrinamicUartDriver.h"
#include "Motors/Dynamixel2.h"
#include "Motors/Servo.h"
#include "Motors/RcServo.h"
#include "Motors/StandardStepper.h"
#include "Motors/UnipolarMotor.h"
#include "Spindles/Spindle.h"
#include "Spindles/NullSpindle.h"
#include "Spindles/DacSpindle.h"
#include "Spindles/10vSpindle.h"
#include "Spindles/BESCSpindle.h"
#include "Spindles/H2ASpindle.h"
#include "Spindles/HuanyangSpindle.h"
#include "Spindles/PWMSpindle.h"
#include "Spindles/RelaySpindle.h"
#include "Spindles/VFDSpindle.h"
#include "Spindles/YL620Spindle.h"
#include "Spindles/Laser.h"
#include "WebUI/WebSettings.h"

using namespace Motors;
using namespace Spindles;
using namespace WebUI;

#define p(...)                                                                                                                             \
    do {                                                                                                                                   \
        grbl_sendf(CLIENT_SERIAL, __VA_ARGS__);                                                                                            \
    } while (0);
const char* tf(bool v) {
    return v ? "true" : "false";
};
static int indent = 0;
void       print_indent() {
    for (int i = 0; i < indent; ++i) {
        p(" ");
    }
}
void section(const char* name) {
    print_indent();
    p("%s:\n", name);
    indent += 2;
}
void end_section() {
    indent -= 2;
}
void item(const char* name, int value) {
    print_indent();
    p("%s: %d\n", name, value);
}
void item(const char* name, unsigned int value) {
    print_indent();
    p("%s: %u\n", name, value);
}
void item(const char* name, bool value) {
    print_indent();
    p("%s: %s\n", name, value ? "true" : "false");
}
void item(const char* name, const char* value) {
    print_indent();
    p("%s: %s\n", name, value);
}
void item(const char* name, float value) {
    print_indent();
    p("%s: %0.3f\n", name, value);
}

const char* pinspec(int pin_number, bool active_low = false, bool pullup = false) {
    if (pin_number == -1) {
        return "UNSPECIFIED";
    }
    if (pin_number == UNDEFINED_PIN) {
        return "NO_PIN";
    }
    static char temp[50];
    temp[0] = '\0';
    if (pin_number < I2S_OUT_PIN_BASE) {
        sprintf(temp, "gpio.%d", pin_number);
    } else {
        sprintf(temp, "i2so.%d", pin_number - I2S_OUT_PIN_BASE);
    }
    if (active_low) {
        strcat(temp, ":low");
    }
    if (pullup) {
        strcat(temp, ":pu");
    }
    return temp;
}
void print_uart(int portnum, uint8_t txd, uint8_t rxd, uint8_t rts, uint32_t baudrate, const char* mode) {
    section("uart");
    item("txd_pin", pinspec(txd));
    item("rxd_pin", pinspec(rxd));
    item("rts_pin", pinspec(rts));
    item("cts_pin", pinspec(UNDEFINED_PIN));  // Nothing in the old code uses CTS
    item("baud", baudrate);
    item("mode", mode);
    end_section();
}

void print_steps(int axis) {
    item("steps_per_mm", axis_settings[axis]->steps_per_mm->get());
    item("max_rate", axis_settings[axis]->max_rate->get());
    item("acceleration", axis_settings[axis]->acceleration->get());
    item("max_travel", axis_settings[axis]->max_travel->get());
    item("soft_limits", tf(soft_limits->get()));
}
void print_homing(int axis) {
    int cycle;
    for (cycle = 0; cycle < MAX_N_AXIS; cycle++) {
        if (bitnum_istrue(homing_cycle[cycle]->get(), axis)) {
            break;
        }
    }
    if (cycle == MAX_N_AXIS) {
        return;
    }
    section("homing");

    item("cycle", cycle + 1);
    item("positive_direction", bitnum_istrue(homing_dir_mask->get(), axis));
    item("mpos", axis_settings[axis]->home_mpos->get());
    item("debounce", homing_debounce->get());
    item("pulloff", homing_pulloff->get());
    item("square", bitnum_istrue(homing_squared_axes->get(), axis));
    item("seek_rate", homing_seek_rate->get());
    item("feed_rate", homing_feed_rate->get());

    float hass = 1.1f;
#ifdef HOMING_AXIS_SEARCH_SCALAR
    hass = HOMING_AXIS_SEARCH_SCALAR;
#endif
    item("seek_scaler", hass);

    float hals = 5.0f;
#ifdef HOMING_AXIS_LOCATE_SCALAR
    hals = HOMING_AXIS_LOCATE_SCALAR;
#endif
    item("feed_scaler", hals);
    end_section();
}
void print_endstops(int axis, int gang) {
    if (limit_pins[axis][gang] != UNDEFINED_PIN) {
        section("endstops");
        bool dlppu = false;
#ifdef DISABLE_LIMIT_PIN_PULL_UP
        dlppu = true;
#endif
        item("dual", pinspec(limit_pins[axis][gang], limit_invert->get(), dlppu));
        item("hard_limits", bool(DEFAULT_HARD_LIMIT_ENABLE));
        end_section();
    }
}
void print_motor(Motor* m, int axis, int gang, const char* name) {
    section(name);
}
void print_servo(Motor* m, int axis, int gang, const char* name = "servo") {
    print_motor(m, axis, gang, name);
}
void print_rc_servo(RcServo* m, int axis, int gang, const char* name = "rc_servo") {
    print_servo(m, axis, gang, name);
    item("cal_min", m->rc_servo_cal_min->get());
    item("cal_max", m->rc_servo_cal_max->get());
    item("pwm", pinspec(m->_pwm_pin, false));
    end_section();
}
#if 0
void print_solenoid(Solenoid* m, int axis, int gang, const char* name = "solenoid") {
    print_rc_servo(m, axis, gang, name);
    item("transition_point", 1.0f);  // Placeholder
    end_section();
}
#endif
void print_unipolar(UnipolarMotor* m, int axis, int gang, const char* name = "unipolar") {
    print_motor(m, axis, gang, name);
    item("phase0", pinspec(m->_pin_phase0, false));
    item("phase1", pinspec(m->_pin_phase1, false));
    item("phase2", pinspec(m->_pin_phase2, false));
    item("phase3", pinspec(m->_pin_phase3, false));
    item("half_step", tf(m->_half_step));
    end_section();
}
void print_dynamixel(Dynamixel2* m, int axis, int gang, const char* name = "dynamixel2") {
    print_servo(m, axis, gang, name);
    item("invert_direction", bitnum_istrue(dir_invert_mask->get(), axis));
    item("count_min", m->_dxl_count_min);
    item("count_max", m->_dxl_count_max);
    item("id", m->ids[axis][gang]);
    print_uart(m->_uart_num, m->_tx_pin, m->_rx_pin, m->_rts_pin, DYNAMIXEL_BAUD_RATE, "8n1");
    end_section();
}
void print_stepper(StandardStepper* m, int axis, int gang, const char* name) {
    print_motor(m, axis, gang, name);
    item("direction", pinspec(m->_dir_pin, m->_invert_dir_pin));
    item("step", pinspec(m->_step_pin, m->_invert_step_pin));
    item("disable", pinspec(m->_disable_pin, false));
}
void print_stepstick(StandardStepper* m, int axis, int gang, const char* name) {
    print_stepper(m, axis, gang, name);
    end_section();
}
const char* trinamicModes(TrinamicMode mode) {
    const char* ret;
    switch (mode) {
        case TrinamicMode::None:
            ret = "ERROR";
        case TrinamicMode::CoolStep:
            ret = "CoolStep";
        case TrinamicMode::StealthChop:
            ret = "StealthChop";
        case TrinamicMode::StallGuard:
            ret = "Stallguard";
    }
    return ret;
}
void print_trinamic_common(int axis, int gang, TrinamicMode run, TrinamicMode homing) {
    item("run_current", axis_settings[axis]->run_current->get());
    item("hold_current", axis_settings[axis]->hold_current->get());
    item("microsteps", axis_settings[axis]->microsteps->get());
    item("stallguard", axis_settings[axis]->stallguard->get());
    item("stallguardDebugMode", false);
    item("run_mode", trinamicModes(run));
    item("homing_mode", trinamicModes(homing));
#ifdef USE_TRINAMIC_ENABLE
    item("use_enable", tf(true));
    item("toff_disable", TRINAMIC_TOFF_DISABLE);
    item("toff_stealthchop", TRINAMIC_TOFF_STEALTHCHOP);
    item("toff_coolstep", TRINAMIC_TOFF_COOLSTEP);
#else
    item("use_enable", tf(false));
#endif
}
void print_trinamic_spi(TrinamicDriver* m, int axis, int gang, const char* name = "trinamic_spi") {
    print_stepper(m, axis, gang, name);
    print_trinamic_common(axis, gang, TRINAMIC_RUN_MODE, TRINAMIC_HOMING_MODE);
    item("r_sense", m->_r_sense);
    item("cs", pinspec(m->_cs_pin, true));
    end_section();
}
void print_trinamic_uart(TrinamicUartDriver* m, int axis, int gang, const char* name = "trinamic_uart") {
    print_stepper(m, axis, gang, name);
    item("r_sense", m->_r_sense);
    print_trinamic_common(axis, gang, TrinamicMode(TRINAMIC_UART_RUN_MODE), TrinamicMode(TRINAMIC_UART_HOMING_MODE));
    print_uart(TMC_UART, TMC_UART_TX, TMC_UART_RX, UNDEFINED_PIN, 115200, "8n1");
    end_section();
}
void print_null_motor(Motor* m, int axis, int gang, const char* name) {
    print_motor(m, axis, gang, name);
    end_section();
}
void print_motor_class(int axis, int gang) {
    Motor*      m    = myMotor[axis][gang];
    const char* name = m->name();
    if (!strcasecmp(name, "null_motor")) {
        print_null_motor(m, axis, gang, name);
        return;
    }
    if (!strcasecmp(name, "stepstick")) {
        print_stepstick(static_cast<StandardStepper*>(m), axis, gang, name);
        return;
    }
#if 0
    if (!strcasecmp(name, "solenoid")) {
        print_solenoid(static_cast<Solenoid*>(m), axis, gang, name);
        return;
    }
#endif
    if (!strcasecmp(name, "rc_servo")) {
        print_rc_servo(static_cast<RcServo*>(m), axis, gang, name);
        return;
    }
    if (!strcasecmp(name, "dynamixel2")) {
        print_dynamixel(static_cast<Dynamixel2*>(m), axis, gang, name);
        return;
    }
    if (!strcasecmp(name, "unipolar")) {
        print_unipolar(static_cast<UnipolarMotor*>(m), axis, gang, name);
        return;
    }
    if (!strcasecmp(name, "tmc_2130") || !strcasecmp(name, "tmc_5160")) {
        print_trinamic_spi(static_cast<TrinamicDriver*>(m), axis, gang, name);
        return;
    }
    if (!strcasecmp(name, "tmc_2208") || !strcasecmp(name, "tmc_2209")) {
        print_trinamic_uart(static_cast<TrinamicUartDriver*>(m), axis, gang, name);
        return;
    }
}
const char* axis_names[] = { "x", "y", "z", "a", "b", "c" };
void        print_axes() {
    section("axes");
    int n_axis = number_axis->get();
    item("number_axis", n_axis);
    item("shared_stepper_disable", STEPPERS_DISABLE_PIN);
    for (int axis = 0; axis < n_axis; axis++) {
        section(axis_names[axis]);
        print_steps(axis);
        print_homing(axis);
        for (int gang = 0; gang < 2; gang++) {
            print_endstops(axis, gang);
            section(gang ? "gang1" : "gang0");
            print_endstops(axis, gang);
            print_motor_class(axis, gang);
            end_section();
        }
        end_section();
    }
    end_section();
}
void print_stepping() {
    section("stepping");
    item("engine", stepper_names[current_stepper]);
    item("idle_ms", stepper_idle_lock_time->get());
    item("pulse_us", pulse_microseconds->get());
    item("dir_delay_us", direction_delay_microseconds->get());
    item("disable_delay_us", enable_delay_microseconds->get());
    end_section();
}

void print_i2so() {
#ifdef USE_I2S_OUT
    section("i2so");
    item("bck", pinspec(I2S_OUT_BCK));
    item("data", pinspec(I2S_OUT_DATA));
    item("ws", pinspec(I2S_OUT_WS));
    end_section();
#endif
}
void print_spi() {
    section("spi");
    // -1 is not the same as UNDEFINED_PIN; -1 means use the hardware default
    item("cs", pinspec(GRBL_SPI_SS == -1 ? GPIO_NUM_5 : GRBL_SPI_SS));
    item("miso", pinspec(GRBL_SPI_MISO == -1 ? GPIO_NUM_19 : GRBL_SPI_MISO));
    item("mosi", pinspec(GRBL_SPI_MOSI == -1 ? GPIO_NUM_23 : GRBL_SPI_MOSI));
    item("sck", pinspec(GRBL_SPI_SCK == -1 ? GPIO_NUM_18 : GRBL_SPI_SCK));
    end_section();
}
#ifndef CONTROL_SAFETY_DOOR_PIN
#    define CONTROL_SAFETY_DOOR_PIN UNDEFINED_PIN
#endif
#ifndef CONTROL_RESET_PIN
#    define CONTROL_RESET_PIN UNDEFINED_PIN
#endif
#ifndef CONTROL_FEED_HOLD_PIN
#    define CONTROL_FEED_HOLD_PIN UNDEFINED_PIN
#endif
#ifndef CONTROL_CYCLE_START_PIN
#    define CONTROL_CYCLE_START_PIN UNDEFINED_PIN
#endif
#ifndef MACRO_BUTTON_0_PIN
#    define MACRO_BUTTON_0_PIN UNDEFINED_PIN
#endif
#ifndef MACRO_BUTTON_1_PIN
#    define MACRO_BUTTON_1_PIN UNDEFINED_PIN
#endif
#ifndef MACRO_BUTTON_2_PIN
#    define MACRO_BUTTON_2_PIN UNDEFINED_PIN
#endif
#ifndef MACRO_BUTTON_3_PIN
#    define MACRO_BUTTON_3_PIN UNDEFINED_PIN
#endif

void print_control() {
    bool pu = true;
#ifdef DISABLE_CONTROL_PIN_PULL_UP
    pu = false;
#endif
    section("control");
    item("safety_door", pinspec(CONTROL_SAFETY_DOOR_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 0), pu));
    item("reset", pinspec(CONTROL_RESET_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 1), pu));
    item("cycle_start", pinspec(CONTROL_CYCLE_START_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 3), pu));
    item("macro0", pinspec(MACRO_BUTTON_0_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 4), pu));
    item("macro1", pinspec(MACRO_BUTTON_1_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 5), pu));
    item("macro2", pinspec(MACRO_BUTTON_2_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 6), pu));
    item("macro3", pinspec(MACRO_BUTTON_3_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 7), pu));
    end_section();
}
#ifndef COOLANT_FLOOD_PIN
#    define COOLANT_FLOOD_PIN UNDEFINED_PIN
#endif
#ifndef COOLANT_MIST_PIN
#    define COOLANT_MIST_PIN UNDEFINED_PIN
#endif
void print_coolant() {
    section("coolant");
    bool floodlow = false;
#ifdef INVERT_COOLANT_FLOOD_PIN
    floodlow = true;
#endif
    item("flood", pinspec(COOLANT_FLOOD_PIN, floodlow, floodlow));
    bool mistlow = false;
#ifdef INVERT_COOLANT_MIST_PIN
    mistlow = true;
#endif
    item("mist", pinspec(COOLANT_MIST_PIN, mistlow, mistlow));
    item("delay_ms", coolant_start_delay->get() * 1000);
    end_section();
}
void print_probe() {
    section("probe");
    bool pu = true;
#ifdef DISABLE_PROBE_PIN_INPUT_PULLUP
    pu = false;
#endif
    item("pin", pinspec(PROBE_PIN, probe_invert->get(), pu));
    bool cms = false;
#ifdef SET_CHECK_MODE_PROBE_TO_START
    cms = true;
#endif
    item("check_mode_start", cms);
    end_section();
}
void print_ips(const char* ssid, const char* ip, const char* gateway, const char* netmask, bool dhcp) {
    item("ssid", ssid);
    item("ip_address", ip);
    item("gateway", gateway);
    item("netmask", netmask);
    item("dhcp", dhcp);
}
void print_ap() {
#ifdef ENABLE_WIFI
    section("wifi_ap");
    print_ips(wifi_ap_ssid->get(), wifi_ap_ip->getStringValue(), "", "", true);
    item("channel", wifi_ap_channel->get());
    end_section();
#endif
}
void print_sta() {
#ifdef ENABLE_WIFI
    section("wifi_sta");
    print_ips(wifi_sta_ssid->get(),
              wifi_sta_ip->getStringValue(),
              wifi_sta_gateway->getStringValue(),
              wifi_sta_netmask->getStringValue(),
              wifi_sta_mode->get() == DHCP_MODE);
    end_section();
#endif
}
void print_bt() {
#ifdef ENABLE_BLUETOOTH
    section("bluetooth");
    item("name", bt_name->get());
    end_section();
#endif
}
void print_comms() {
    // Radio mode???

    section("comms");
#ifdef ENABLE_WIFI
    item("telnet_enable", telnet_enable->get());
    item("telnet_port", telnet_port->get());

    item("http_enable", http_enable->get());
    item("http_port", http_port->get());

    item("hostname", wifi_hostname->get());

    print_ap();
    print_sta();
#endif
#ifdef ENABLE_BLUETOOTH
    print_bt();
#endif
    end_section();
}
// notifications?
void print_macros() {
    section("macros");
    item("n0", startup_line_0->get());
    item("n1", startup_line_1->get());
    item("macro0", user_macro0->get());
    item("macro1", user_macro1->get());
    item("macro2", user_macro2->get());
    item("macro3", user_macro3->get());
    end_section();
}
const char* makeSpeedMap(PWM* s) {
    static char temp[100];
    float       off_percent = spindle_pwm_off_value->get();
    float       min_percent = spindle_pwm_min_value->get();
    float       max_percent = spindle_pwm_max_value->get();
    float       min_rpm     = rpm_min->get();
    float       max_rpm     = rpm_max->get();
    if (min_rpm == 0.0f && off_percent == min_percent) {
        sprintf(temp, "%d=%.1f%% %d=%.1f%%", (int)min_rpm, min_percent, (int)max_rpm, max_percent);
    } else {
        sprintf(temp, "%d=%.1f%% %d=%.1f%% %d=%.1f%%", 0, off_percent, (int)min_rpm, min_percent, (int)max_rpm, max_percent);
    }
    return temp;
}
void print_spindle(const char* name, Spindle* s) {
    section(name);
    item("spinup_ms", s->_spinup_delay);
    item("spindown_ms", s->_spindown_delay);
    item("tool", int(0));
    item("speeds", makeSpeedMap(static_cast<PWM*>(s)));
    end_section();
}
void print_onoff_spindle(const char* name, PWM* s) {
    print_spindle(name, s);
    item("output_pin", pinspec(s->_output_pin, s->_invert_pwm, false));
    item("enable_pin", pinspec(s->_enable_pin));
    item("direction_pin", pinspec(s->_direction_pin));
    item("disable_with_zero_speed", bool(s->_off_with_zero_speed));
    item("zero_speed_with_disable", false);
}
void print_pwm_spindle(const char* name, PWM* s) {
    print_onoff_spindle(name, s);
    item("pwm_freq", s->_pwm_freq);
    end_section();
}
void print_relay_spindle(Relay* s) {
    print_onoff_spindle("relay", s);
    end_section();
}

void print_laser_spindle(Laser* s) {
    print_pwm_spindle("laser", s);
    end_section();
}

void print_dac_spindle(Dac* s) {
    print_onoff_spindle("dac", s);
    end_section();
}
void print_besc_spindle(BESC* s) {
    s->_pwm_freq = BESC_PWM_FREQ;  // Override in parent class
    print_pwm_spindle("besc", s);
    item("min_pulse_us", int(BESC_MIN_PULSE_SECS * 1000000));
    item("max_pulse_us", int(BESC_MAX_PULSE_SECS * 1000000));

    end_section();
}

void print_10v_spindle(_10v* s) {
    print_pwm_spindle("10v", s);
    end_section();
}

void print_vfd_spindle(const char* name, VFD* s) {
    print_spindle(name, s);
    print_uart(VFD_RS485_UART_PORT,
#ifdef VFD_RS485_TXD_PIN
               VFD_RS485_TXD_PIN
#else
               -1
#endif
               ,
#ifdef VFD_RS485_RXD_PIN
               VFD_RS485_RXD_PIN
#else
               -1
#endif
               ,
#ifdef VFD_RS485_RTS_PIN
               VFD_RS485_RTS_PIN
#else
               -1
#endif
               ,
#ifdef VFD_RS485_BAUD_RATE
               VFD_RS485_BAUD_RATE
#else
               9600
#endif
               ,

#ifdef VFD_RS485_PARITY
#    if VFD_RS485_PARITY == Uart::Parity::None
               "8n1"
#        elseif VFD_RS485_PARITY == Uart::Parity::Even
               "8e1"
#    else  // VFD_RS485_PARITY == Uart::Parity::Odd
               "8o1"
#    endif
#else
               "8n1"
#endif
    );
    end_section();
}

void print_huanyang_spindle(Huanyang* s) {
    print_vfd_spindle("huanyang", s);
    end_section();
}

void print_h2a_spindle(H2A* s) {
    print_vfd_spindle("h2a", s);
    end_section();
}

void print_yl620_spindle(YL620* s) {
    print_vfd_spindle("yl620", s);
    end_section();
}

void print_spindle_class() {
    Spindle* s = spindle;
    switch (spindle_type->get()) {
        case int8_t(SpindleType::NONE):
            break;
        case int8_t(SpindleType::PWM):
            print_pwm_spindle("pwm", static_cast<PWM*>(s));
            break;
        case int8_t(SpindleType::RELAY):
            print_relay_spindle(static_cast<Relay*>(s));
            break;
        case int8_t(SpindleType::LASER):
            print_laser_spindle(static_cast<Laser*>(s));
            break;
        case int8_t(SpindleType::DAC):
            print_dac_spindle(static_cast<Dac*>(s));
            break;
        case int8_t(SpindleType::HUANYANG):
            print_huanyang_spindle(static_cast<Huanyang*>(s));
            break;
        case int8_t(SpindleType::BESC):
            print_besc_spindle(static_cast<BESC*>(s));
            break;
        case int8_t(SpindleType::_10V):
            print_10v_spindle(static_cast<_10v*>(s));
            break;
        case int8_t(SpindleType::H2A):
            print_h2a_spindle(static_cast<H2A*>(s));
            break;
        case int8_t(SpindleType::YL620):
            print_yl620_spindle(static_cast<YL620*>(s));
            break;
    }
}
void dump_config() {
#ifdef USE_I2S_OUT
    item("board", "6-pack");
#else
    item("board", "unknown");
#endif
    item("name", MACHINE_NAME);
    print_stepping();
    print_axes();
    print_i2so();
    print_spi();
    print_control();
    print_coolant();
    print_probe();
    print_comms();
    print_macros();

    int db = 0;
#ifdef ENABLE_SOFTWARE_DEBOUNCE
    db = DEBOUNCE_PERIOD;
#endif
    item("software_debounce_ms", db);
    // TODO: Consider putting these under a gcode: hierarchy level? Or motion control?
    item("laser_mode", laser_mode->get());
    item("arc_tolerance", arc_tolerance->get());
    item("junction_deviation", junction_deviation->get());
    item("verbose_errors", verbose_errors->get());

    bool hil = false;
#ifdef HOMING_INIT_LOCK
    hil = true;
#endif
    item("homing_init_lock", tf(hil));

    item("report_inches", report_inches->get());

    bool epoc = false;
#ifdef ENABLE_PARKING_OVERRIDE_CONTROL
    epoc = true;
#endif
    item("enable_parking_override_control", tf(epoc));

    bool dpoi = false;
#ifdef DEACTIVATE_PARKING_UPON_INIT
    dpoi = true;
#endif
    item("deactivate_parking_upon_init", tf(dpoi));

    bool clai = false;
#ifdef CHECK_LIMITS_AT_INIT
    clai = true;
#endif
    item("check_limits_at_init", tf(clai));

    bool l2soa = false;
#ifdef LIMITS_TWO_SWITCHES_ON_AXES
    l2soa = true;
#endif
    item("limits_two_switches_on_axis", tf(l2soa));

    bool dldh = false;
#ifdef DISABLE_LASER_DURING_HOLD
    dldh = true;
#endif
    item("disable_laser_during_hold", tf(dldh));

    bool uln = false;
#ifdef USE_LINE_NUMBERS
    uln = true;
#endif
    item("use_line_numbers", uln);

    print_spindle_class();
}
#if 0
void machine_init() {
    print_machine();
}
#endif

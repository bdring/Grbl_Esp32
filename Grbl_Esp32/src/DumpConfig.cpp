#define p(...)                                                                                                                             \
    do {                                                                                                                                   \
        grbl_sendf(CLIENT_SERIAL, __VA_ARGS__);                                                                                            \
        grbl_sendf(CLIENT_SERIAL, "\n");                                                                                                   \
    } while (0);
const char* tf(bool v) { return v ? "true" : "false" };
void        print_steps(int axis) {
    p("    steps_per_mm: %f", axis_settings[axis]->steps_per_mm->get());
    p("    max_rate: %f", axis_settings[axis]->max_rate->get());
    p("    acceleration: %f", axis_settings[axis]->acceleration->get());
    p("    max_travel: %f", axis_settings[axis]->max_travel->get());
    p("    soft_limits: %s", tf(soft_limits->get()));
}
void print_homing(int axis) {
    for (int cycle; cycle < MAX_N_AXIS; cycle++) {
        if (bitnum_istrue(homing_cycle[cycle], axis)) {
            break;
        }
    }
    if (cycle == MAX_N_AXIS) {
        // XXX alternative: set cycle to 0
        return;
    }
    p("    homing:") p("      cycle", cycle);
    p("      positive_direction: %s", tf(bitnum_istrue(homing_dir_mask->get(), axis)));
    p("      mpos: %f", axis_settings[axis]->home_mpos->get());
    p("      debounce: %f", homing_debounce->get());
    p("      pulloff: %f", homing_pulloff->get());
    p("      square: %s", tf(bitnum_istrue(homing_squared_axes->get, axis));
    p("      seek_rate: %f", homing_seek_rate->get());
    p("      feed_rate: %f", homing_feed_rate->get());
    p("      seek_scaler: %f", HOMING_AXIS_SEARCH_SCALAR);
    p("      feed_scaler: %f", HOMING_AXIS_LOCATE_SCALAR);
}
void print_endstops(int axis, int gang) {
    if (limit_pins[axis][gang] != UNDEFINED_PIN) {
        p("      endstops:");
        p("        dual: %s", print_pin(limit_pins[axis][gang], limit_invert->get(), DISABLE_LIMIT_PIN_PULL_UP));
        p("        hard_limits: %s", tf(DEFAULT_HARD_LIMIT_ENABLE));
    }
}
void print_motor(Motor* m, int axis, int gang) {
    p("      %s:", m->name());
}
void print_servo(Motor* m, int axis, int gang) {
    print_motor(m, axis, gang));
}
void print_rc_servo(Motor* m, int axis, int gang) {
    print_servo(m, axis, gang);
    p("      cal_min: %f", m->_cal_min);
    p("      cal_max: %f", m->_cal_max);
    p("      pwm: %s" print_pin(m->_pwm_pin, false))
}
void print_solenoid(Motor* m, int axis, int gang) {
    print_rc_servo(m, axis, gang);
    p("      transition_point: %f", 1.0f);  // Placeholder
}
void print_unipolar(Motor* m, int axis, int gang) {
    print_motor(m, axis, gang));
    p("      phase0: %s", print_pin(m->_pin_phase0, false));
    p("      phase1: %s", print_pin(m->_pin_phase1, false));
    p("      phase2: %s", print_pin(m->_pin_phase2, false));
    p("      phase3: %s", print_pin(m->_pin_phase3, false));
    p("      half_step: %s", tf(m->_half_step)));
}
void print_dynamixel(Motor* m, int axis, int gang) {
    print_servo(m, axis, gang);
    p("      invert_direction: %s", tf(bitnum_istrue(dir_invert_mask->get(), axis)));
    p("      count_min: %d", int(m->dxl_count_min));
    p("      count_max: %d", int(m->dxl_count_max));
    p("      full_time_move: %d", DYNAMIXEL_FULL_MOVE_TIME);
    p("      id: %d", m->ids[axis][gang]);
    p("      uart:");
    print_uart(m->_uart_num, m->_tx_pin, m->_rx_pin, m->_rts_pin);
}
void print_stepstick(Motor* m, int axis, int gang) {
    print_motor(m, axis, gang);
    p("      direction: %s", print_pin(m->_dir_pin, m->_invert_dir_pin));
    p("      step: %s", print_pin(m->_step_pin, m->_invert_step_pin));
    p("      disable: %s", print_pin(m->_disable_pin, false));
}
const char* trinamicModes(TrinamicMode mode) {
    switch (mode) {
        case TrinamicMode::CoolStep:
            return "CoolStep";
        case TrinamicMode::StealthChop:
            return "StealthChop";
        case TrinamicMode::Stallguard:
            return "Stallguard";
    }
}
void print_trinamic_base(Motor* m, int axis, int gang) {
    print_stepstick(m, axis, gang);
    p("       r_sense: %f", m->_r_sense);
    p("       run_current: %f", axis_settings[axis]->run_current->get());
    p("       hold_current: %f", axis_settings[axis]->hold_current->get());
    p("       microsteps: %d", axis_settings[axis]->microsteps->get());
    p("       stallguard: %d", axis_settings[axis]->stallguard->get());
    p("       stallguardDebugMode: %s", tf(false));
    p("       run_mode: %s", _run_mode, trinamicModes(TRINAMIC_RUN_MODE));
    p("       homing_mode: %s", _homing_mode, trinamicModes(TRINAMIC_HOMING_MODE));
#ifdef USE_TRINAMIC_ENABLE
    p("       use_enable: %s", tf(true));
    p("       toff_disable: %d", TRINAMIC_TOFF_DISABLE);
    p("       toff_stealthchop: %d", TRINAMIC_TOFF_STEALTHCHOP);
    p("       toff_coolstep: %d", TRINAMIC_TOFF_COOLSTEP);
#else
    p("       use_enable: %s", tf(false));
#endif
}
void print_trinamic_spi(Motor* m, int axis, int gang) {
    print_trinamic_base(m, axis, gang);
    p("      cs: %s", print_pin(m->_cs_pin, true));
}
void print_trinamic_uart(Motor* m, int axis, int gang) {
    print_trinamic_base(m, axis, gang);
    p("      uart:");
    print_uart(???);
}
void print_motor_class(Motor* m, int axis, int gang) {
    const char* name = m->name();
    if (!strcmp(name, "null_motor")) {
        print_motor_name(name);
        return;
    }
    if (!strcmp(name, "stepstick")) {
        print_stepstick(m, axis, gang);
        return;
    }
    if (!strcmp(name, "solenoid")) {
        print_solenoid(name);
        return;
    }
    if (!strcmp(name, "rc_servo")) {
        print_rc_servo(name);
        return;
    }
    if (!strcmp(name, "dynamixel2")) {
        print_dynamixel(m, axis, gang);
        return;
    }
    if (!strcmp(name, "unipolar")) {
        print_unipolar(name);
        return;
    }
    if (!strcmp(name, "tmc_2130" || !strcmp(name, "tmc_5160")) {
        print_trinamic_spi(name);
        return;
    }
    if (!strcmp(name, "tmc_2208" || !strcmp(name, "tmc_2209")) {
        print_trinamic_uart(name);
        return;
    }
}
void print_axes() {
    p("axes:");
    for (int axis = 0; axis <= n_axis; axis++) {
        p("  %c:", "xyzabc"[axis]);
        print_steps(axis);
        print_homing(axis);
        for (int gang = 0; gang < 2; gang++) {
            print_endstops(axis);
            p("    gang%d:", gang);
            print_endstops(axis, gang);
            print_motor_class(myMotor[axis][gang], axis, gang);
        }
    }
}
void print_stepping() {
    p("  engine: %s", stepper_names[current_stepper]);
    p("  idle_ms: %d", stepper_idle_lock_time);
    p("  pulse_us: %d", pulse_microseconds);
    p("  dir_delay_us: %d", direction_delay_microseconds);
    p("  disable_delay_us: %d", enable_delay_microseconds);
}

void print_i2so() {
    p("i2so:");
    p("  bck: %s", print_pin(_bck, );
    p("  data: %s", print_pin(_data, );
    p("  ws: %s", print_pin(_ws, );
}
void print_spi() {
    p("spi:");
    p("  cs: %s", print_pin(_cs, );
    p("  miso %s", print_pin(_miso, );
    p("  mosi %s", print_pin(_mosi, );
    p("  sck %s", print_pin(_sck, );
}
void print_control() {
    p("control:");
    p("  safety_door: %s", print_pin(CONTROL_SAFETY_DOOR_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 0), DISABLE_CONTROL_PIN_PULL_UP);
    p("  reset: %s", print_pin(CONTROL_RESET_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 1), DISABLE_CONTROL_PIN_PULL_UP);
    p("  feed_hold: %s", print_pin(CONTROL_FEED_HOLD_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 2), DISABLE_CONTROL_PIN_PULL_UP);
    p("  cycle_start: %s", print_pin(CONTROL_CYCLE_START_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 3), DISABLE_CONTROL_PIN_PULL_UP);
    p("  macro0: %s", print_pin(MACRO_BUTTON_0_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 4), DISABLE_CONTROL_PIN_PULL_UP);
    p("  macro1: %s", print_pin(MACRO_BUTTON_1_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 5), DISABLE_CONTROL_PIN_PULL_UP);
    p("  macro2: %s", print_pin(MACRO_BUTTON_2_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 6), DISABLE_CONTROL_PIN_PULL_UP);
    p("  macro3: %s", print_pin(MACRO_BUTTON_3_PIN, bitnum_istrue(INVERT_CONTROL_PIN_MASK, 7), DISABLE_CONTROL_PIN_PULL_UP);
}
void print_coolant() {
    p("coolant:");
    p("  flood: %s", print_pin(COOLANT_FLOOD_PIN, INVERT_COOLANT_FLOOD_PIN));
    p("  mist: %s", print_pin(COOLANT_MIST_PIN, INVERT_COOLANT_MIST_PIN));
    p("  delay: %f", coolant_start_delay->get());  // XXX should the be int as delay_ms?
}
void print_probe() {
    p("probe:");
    p("  pin: %s", print_pin(PROBE_PIN, probe_invert->get(), DISABLE_PROBE_PIN_INPUT_PULLUP);
    bool cms = false;
#ifdef SET_CHECK_MODE_PROBE_TO_START
    cms = true;
#endif
    p("  check_mode_start: %s", cms);
}
void print_comms() {
    p("comms:");
    // XXX
}
void print_macros() {
    p("macros:");
    p("  n0: %s", startup_line0->get());
    p("  n1: %s", startup_line1->get());
    p("  macro0: %s", user_macro0->get());
    p("  macro1: %s", user_macro1->get());
    p("  macro2: %s", user_macro2->get());
    p("  macro3: %s", user_macro3->get());
}
void print_spindle(const char* name, Spindle* s) {
    p("%s:", name);
    p("  spinup_ms: %d", s->_spinup_delay);
    p("  spindown_ms: %d", s->_spindown_delay);
    p("  tool: 0");
    p("  speeds: %s", makeSpeedMap(s));
}
void print_onoff_spindle(const char* name, Spindle* s) {
    print_spindle(name, s);
    p("  output_pin: %s", print_pin(s->_output_pin));
    p("  enable_pin: %s", print_pin(s->_enable_pin));
    p("  direction_pin: %s", print_pin(s->_direction_pin));
    p("  disable_with_zero_speed: %s", tf(s->_off_with_zero_speed));
    p("  zero_speed_with_disable: %s", tf(false));
}
void print_pwm_spindle(const char* name = "pwm", Spindle* s = &pwmb) {
    print_onoff_spindle(name, s);
    p("  pwm_freq: %d", s-?_pwm_freq)''
}
void print_relay_spindle() {
    Spindle* s = &relay;
    print_onoff_spindle("relay", s);
}

void print_laser_spindle() {
    print_pwm_spindle("laser", s);
    // XXX should this be just full_power ?
    p("  laser_full_power: %u", laser_full_power->get());
}

void print_dac_spindle() {
    print_onoff_spindle("dac", &dac);
}

void print_besc_spindle() {
    Spindle* s = &besc;
    print_pwm_spindle("besc", s);
    // XXX override frequency to BEDC_PWM_FREQ and period to BESC_PULSE_PERIOD
    p("  min_pulse_us: %d", int(BESC_MIN_PULSE_SECS * 1000000));
    p("  max_pulse_us: %d", int(BESC_MAX_PULSE_SECS * 1000000))
}

void print_10v_spindle() {
    Spindle* s = &_10v;
    print_pwm_spindle("10v", s);
}

void print_vfd_spindle(const char* name, Spindle* s) {
    print_spindle(name, s);
    print_uart(VFD_RS485_UART_PORT,
#ifdef VFD_RS485_TXD_PIN VFD_RS485_TXD_PIN
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
#    else if VFD_RS485_PARITY == Uart::Parity::Even
               "8e1"
#    else if VFD_RS485_PARITY == Uart::Parity::Odd
               "8o1"
#    endif
#else
               "8n1"
#endif
    );
}

void print_huanyang_spindle() {
    print_vfd_spindle("huanyang", &huanyang);
}

void print_h2a_spindle() {
    print_vfd_spindle("h2a", &h2a);
}

void print_yl620_spindle() {
    print_vfd_spindle("yl620", &yl620);
}

void print_spindle_class() {
    switch (spindle_type->get()) {
        case SpindleType::NONE:
            break;
        case SpindleType::PWM:
            print_pwm_spindle();
            break;
        case SpindleType::RELAY:
            print_relay_spindle();
            break;
        case SpindleType::LASER:
            print_laser_spindle();
            break;
        case SpindleType::DAC:
            print_dac_spindle();
            break;
        case SpindleType::HUANYANG:
            print_huanyang_spindle();
            break;
        case SpindleType::BESC:
            print_besc_spindle();
            break;
        case SpindleType::_10V:
            print_10v_spindle();
            break;
        case SpindleType::H2A:
            print_h2a_spindle();
            break;
        case SpindleType::YL620:
            print_yl620_spindle();
            break;
    }
}
void print_machine() {
    p("board", _board);
    p("name", _name);
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
    p("software_debounce_ms", db);
    // TODO: Consider putting these under a gcode: hierarchy level? Or motion control?
    p("laser_mode: %", laser_mode->get());
    p("arc_tolerance: %", arc_tolerance->get());
    p("junction_deviation: %", junction_deviation->get());
    p("verbose_errors: %", verbose_errors->get());

    bool hil = false;
#ifdef HOMING_INIT_LOCK
    hil = true;
#endif
    p("homing_init_lock: %s", tf(hil));

    p("report_inches: %", report_inches->get());

    bool epoc = false;
#ifdef ENABLE_PARKING_OVERRIDE_CONTROL
    epoc = true;
#endif
    p("enable_parking_override_control: %", tf(epoc));

    bool dpoi = false;
#ifdef DEACTIVATE_PARKING_UPON_INIT
    dpoi = true;
#endif
    p("deactivate_parking_upon_init: %", tf(dpoi));

    bool clai = false;
#ifdef CHECK_LIMITS_AT_INIT
    clai = true;
#endif
    p("check_limits_at_init: %", tf(clai));

    bool l2soa = false;
#ifdef LIMITS_TWO_SWITCHES_ON_AXES
    l2soa = true;
#endif
    p("limits_two_switches_on_axis: %", tf(l2soa));

    bool dldh = false;
#ifdef DISABLE_LASER_DURING_HOLD
    dldh = true;
#endif
    p("disable_laser_during_hold: %", tf(dldh));

    bool uln = false;
#ifdef USE_LINE_NUMBERS
    uln = true;
#endif
    p("use_line_numbers: %", tf(uln);

    print_spindle_class();
}
void machine_init() {
    print_machine();
}

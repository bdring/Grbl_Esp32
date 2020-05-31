void motors_set_disable(bool disable) {
    static bool previous_state = false;

    if (previous_state == disable)
        return;

    previous_state = disable;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "motors_set_disable:%d", disable);


#ifdef USE_TRINAMIC_ENABLE
    trinamic_stepper_enable(!isOn);
#endif
    if (bit_istrue(settings.flags, BITFLAG_INVERT_ST_ENABLE)) {
        disable = !disable;    // Apply pin invert.
    }
#ifdef USE_UNIPOLAR
    unipolar_disable(isOn);
#endif
#ifdef STEPPERS_DISABLE_PIN
    HAL_digitalWrite(X_ENABLE_PIN, disable);
#endif

#ifdef X_ENABLE_PIN
    HAL_digitalWrite(X_ENABLE_PIN, disable);
#endif
#ifdef Y_ENABLE_PIN
    HAL_digitalWrite(Y_ENABLE_PIN, disable);
#endif
#ifdef Z_ENABLE_PIN
    HAL_digitalWrite(Z_ENABLE_PIN, disable);
#endif
#ifdef A_ENABLE_PIN
    HAL_digitalWrite(A_ENABLE_PIN, disable);
#endif
#ifdef B_ENABLE_PIN
    HAL_digitalWrite(B_ENABLE_PIN, disable);
#endif
#ifdef C_ENABLE_PIN
    HAL_digitalWrite(C_ENABLE_PIN, disable);
#endif
}

void motors_set_direction_pins(uint8_t onMask) {
#ifdef X_DIRECTION_PIN
    HAL_digitalWrite(X_DIRECTION_PIN, (onMask & (1 << X_AXIS)));
#endif
#ifdef X2_DIRECTION_PIN // optional ganged axis
    HAL_digitalWrite(X2_DIRECTION_PIN, (onMask & (1 << X_AXIS)));
#endif
#ifdef Y_DIRECTION_PIN
    HAL_digitalWrite(Y_DIRECTION_PIN, (onMask & (1 << Y_AXIS)));
#endif
#ifdef Y2_DIRECTION_PIN // optional ganged axis
    HAL_digitalWrite(Y2_DIRECTION_PIN, (onMask & (1 << Y_AXIS)));
#endif
#ifdef Z_DIRECTION_PIN
    HAL_digitalWrite(Z_DIRECTION_PIN, (onMask & (1 << Z_AXIS)));
#endif
#ifdef Z2_DIRECTION_PIN // optional ganged axis
    HAL_digitalWrite(Z2_DIRECTION_PIN, (onMask & (1 << Z_AXIS)));
#endif
#ifdef A_DIRECTION_PIN
    HAL_digitalWrite(A_DIRECTION_PIN, (onMask & (1 << A_AXIS)));
#endif
#ifdef A2_DIRECTION_PIN // optional ganged axis
    HAL_digitalWrite(A2_DIRECTION_PIN, (onMask & (1 << A_AXIS)));
#endif
#ifdef B_DIRECTION_PIN
    HAL_digitalWrite(B_DIRECTION_PIN, (onMask & (1 << B_AXIS)));
#endif
#ifdef B2_DIRECTION_PIN // optional ganged axis
    HAL_digitalWrite(B2_DIRECTION_PIN, (onMask & (1 << B_AXIS)));
#endif
#ifdef C_DIRECTION_PIN
    HAL_digitalWrite(C_DIRECTION_PIN, (onMask & (1 << C_AXIS)));
#endif
#ifdef C2_DIRECTION_PIN // optional ganged axis
    HAL_digitalWrite(C2_DIRECTION_PIN, (onMask & (1 << C_AXIS)));
#endif
}


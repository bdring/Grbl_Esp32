#include "Ledc.h"

/*
    Ledc.cpp

    This is a driver for the ESP32 LEDC controller that is similar
    to the Arduino HAL driver for LEDC.  It differs from the Arduino
    driver by being able to handle output pin inversion in hardware,
    and by having the ledcSetDuty function in IRAM so it is safe
    to call it from ISRs.

    Part of Grbl_ESP32

    2021 -	Mitch Bradley

    Grbl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <soc/ledc_struct.h>
#include <driver/ledc.h>

extern "C" void __pinMode(uint8_t pin, uint8_t mode);

void ledcInit(Pin& pin, uint8_t chan, double freq, uint8_t bit_num) {
    ledcSetup(chan, freq, bit_num);  // setup the channel

    auto nativePin = pin.getNative(Pin::Capabilities::PWM);

    // This is equivalent to ledcAttachPin with the addition of
    // using the hardware inversion function in the GPIO matrix.
    // We use that to apply the active low function in hardware.
    __pinMode(nativePin, OUTPUT);
    uint8_t function    = ((chan / 8) ? LEDC_LS_SIG_OUT0_IDX : LEDC_HS_SIG_OUT0_IDX) + (chan % 8);
    bool    isActiveLow = pin.getAttr().has(Pin::Attr::ActiveLow);
    pinMatrixOutAttach(nativePin, function, isActiveLow, false);
}

void IRAM_ATTR ledcSetDuty(uint8_t chan, uint32_t duty) {
    uint8_t g = chan >> 3, c = chan & 7;
    bool    on = duty != 0;
    // This is like ledcWrite, but it is called from an ISR
    // and ledcWrite uses RTOS features not compatible with ISRs
    // Also, ledcWrite infers enable from duty, which is incorrect
    // for use with RcServo which wants the
    LEDC.channel_group[g].channel[c].duty.duty        = duty << 4;
    LEDC.channel_group[g].channel[c].conf0.sig_out_en = on;
    LEDC.channel_group[g].channel[c].conf1.duty_start = on;
}

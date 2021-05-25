#pragma once

#include "Pin.h"

class ControlPin {
private:
    // invertBitNum refers to a bit in INVERT_CONTROL_PIN_MASK.  It is a
    // short-term hack to reduce the extent of the patch.
    bool           _invertBitNum;
    bool           _value;
    const char     _letter;
    volatile bool& _rtVariable;
    const char*    _legend;

    void IRAM_ATTR handleISR();

public:
    ControlPin(uint8_t bitNum, volatile bool& rtVariable, const char* legend, char letter) :
        _invertBitNum(bitNum), _value(false), _letter(letter), _rtVariable(rtVariable), _legend(legend) {
        _rtVariable = _value;
    }

    Pin _pin;

    void        init();
    bool        get() { return _value; }
    const char* legend() { return _legend; }
    //    char        invertBitNum() { return _invertBitNum; }
    char letter() { return _letter; }

    void report(char* status, bool& pinReportStarted);

    ~ControlPin();
};

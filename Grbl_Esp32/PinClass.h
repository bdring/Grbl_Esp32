// Various combinations are valid, e.g
// input|pullup
typedef enum {
    inputPin   =  1,
    outputPin  =  2,
    pullUp     =  4,
    pullDown   =  8,
    openDrain  = 16,
} PinMode_t;

// Will be used for interrupt options, etc.
typedef enum {
    none       =  0,
} PinOptions_t;

// Base class defining abstract operations on pins
class OutPin {
public:
    void set() {};
    void clear() {};
    void write(const bool value) {};
};


class InPin {
public:
    bool read() { return 0; };
};

class GPIOout : public OutPin {
private:
    uint8_t    pinnum;
    PinMode_t  mode;
    Reg_t*     w1ts;
    Reg_t*     w1tc;
    uint32_t   mask;
public:
    GPIOout(uint8_t _pinnum)
    {
        pinnum = _pinnum;
        if (pinnum >= 32) {
            mask = 1 << (pinnum - 32);
            w1ts = (Reg_t*)GPIO.out1_w1ts;
            w1tc = (Reg_t*)GPIO.out1_w1tc;
        } else {
            mask = 1 << pinnum;
            w1ts = (Reg_t*)GPIO.out_w1ts;
            w1tc = (Reg_t*)GPIO.out_w1tc;
        }
        pinMode(pinnum, outputPin);
    }
    void set() { *w1ts = mask; }
    void clear() { *w1tc = mask; }
    void write(bool set) {
        if (set) {
            set();
        } else {
            clear();
        }
    }
}

class GPIOin : public InPin {
private:
    uint8_t    pinnum;
    Reg_t*     in;
    uint32_t   mask;
public:
    GPIOin(uint8_t _pinnum)
    {
        pinnum = _pinnum;
        if (pinnum >= 32) {
            mask = 1 << (_pinnum - 32);
            in   = (Reg_t*)GPIO.in1;
        } else {
            mask = 1 << _pinnum;
            in   = (Reg_t*)GPIO.in;
        }
        pinMode(pinnum, inputPin);
    }
    bool read() {
        return !!(*in & mask);
    }
}

#if 0
class I2SOut : public Pin {
}
class I2SIn : public Pin {
}
#endif

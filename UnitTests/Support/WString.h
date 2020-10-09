#pragma once

#include <WString.h>
#include <string>
#include <iomanip>
#include <sstream>
#pragma warning(disable : 4996)

class StringSumHelper;

class String {
    std::string backbuf;

    static std::string ValueToString(int value, int base) {
        char        buffer[100] = { 0 };
        int         number_base = 10;
        std::string output      = itoa(value, buffer, base);
        return output;
    }

    static std::string DecToString(double value, int decimalPlaces) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(decimalPlaces) << value;
        std::string s = stream.str();
        return s;
    }

    typedef void (String::*StringIfHelperType)() const;
    void StringIfHelper() const {}

    String(std::string& s) : backbuf(s) {}

protected:
    inline const char* buffer() const { return backbuf.c_str(); }
    inline char* wbuffer() { return &*backbuf.begin(); }

public:
    String(const char* cstr = "") : backbuf(cstr) {}
    String(const String& str) : backbuf(str.backbuf) {}
    explicit String(char c) : backbuf({ c }) {}
    explicit String(unsigned char ch, unsigned char base = 10) : backbuf(ValueToString(ch, base)) {}
    explicit String(int ch, unsigned char base = 10) : backbuf(ValueToString(ch, base)) {}
    explicit String(unsigned int ch, unsigned char base = 10) : backbuf(ValueToString(ch, base)) {}
    explicit String(long ch, unsigned char base = 10) : backbuf(ValueToString(ch, base)) {}
    explicit String(unsigned long ch, unsigned char base = 10) : backbuf(ValueToString(ch, base)) {}
    explicit String(float fl, unsigned char decimalPlaces = 2) : backbuf(DecToString(fl, decimalPlaces)) {}
    explicit String(double fl, unsigned char decimalPlaces = 2) : backbuf(DecToString(fl, decimalPlaces)) {}

    String& operator=(const String& rhs) { backbuf = rhs.backbuf; return *this; }
    String& operator=(const char* cstr) { backbuf = cstr; return *this; }

    unsigned char reserve(unsigned int size) {
        backbuf.reserve(size);
        return true;
    }
    inline unsigned int length() const { return static_cast<unsigned int>(backbuf.size()); }

    inline void clear() { backbuf = ""; }

    inline bool isEmpty() const { return length() == 0; }

    unsigned char concat(const String& str) {
        backbuf += str.backbuf;
        return true;
    }
    unsigned char concat(const char* cstr) {
        backbuf += String(cstr).backbuf;
        return true;
    }
    unsigned char concat(char c) {
        backbuf += String(c).backbuf;
        return true;
    }

    unsigned char concat(unsigned char c) {
        backbuf += String(c).backbuf;
        return true;
    }

    unsigned char concat(int num) {
        backbuf += String(num).backbuf;
        return true;
    }

    unsigned char concat(unsigned int num) {
        backbuf += String(num).backbuf;
        return true;
    }

    unsigned char concat(long num) {
        backbuf += String(num).backbuf;
        return true;
    }

    unsigned char concat(unsigned long num) {
        backbuf += String(num).backbuf;
        return true;
    }

    unsigned char concat(float num) {
        backbuf += String(num).backbuf;
        return true;
    }

    unsigned char concat(double num) {
        backbuf += String(num).backbuf;
        return true;
    }
    String& operator+=(const String& rhs) {
        concat(rhs);
        return (*this);
    }
    String& operator+=(const char* cstr) {
        concat(cstr);
        return (*this);
    }
    String& operator+=(char c) {
        concat(c);
        return (*this);
    }
    String& operator+=(unsigned char num) {
        concat(num);
        return (*this);
    }
    String& operator+=(int num) {
        concat(num);
        return (*this);
    }
    String& operator+=(unsigned int num) {
        concat(num);
        return (*this);
    }
    String& operator+=(long num) {
        concat(num);
        return (*this);
    }
    String& operator+=(unsigned long num) {
        concat(num);
        return (*this);
    }
    String& operator+=(float num) {
        concat(num);
        return (*this);
    }
    String& operator+=(double num) {
        concat(num);
        return (*this);
    }

    friend StringSumHelper& operator +(const StringSumHelper& lhs, const String& rhs);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, const char* cstr);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, char c);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, unsigned char num);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, int num);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, unsigned int num);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, long num);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, unsigned long num);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, float num);
    friend StringSumHelper& operator +(const StringSumHelper& lhs, double num);

    operator StringIfHelperType() const { return buffer() ? &String::StringIfHelper : 0; }

    int           compareTo(const String& s) const { return backbuf.compare(s.backbuf); }
    unsigned char equals(const String& s) const { return backbuf == s.backbuf; }
    unsigned char equals(const char* cstr) const { return backbuf == cstr; }
    unsigned char operator==(const String& rhs) const { return equals(rhs); }
    unsigned char operator==(const char* cstr) const { return equals(cstr); }
    unsigned char operator!=(const String& rhs) const { return !equals(rhs); }
    unsigned char operator!=(const char* cstr) const { return !equals(cstr); }
    unsigned char operator<(const String& rhs) const { return compareTo(rhs) < 0; }
    unsigned char operator>(const String& rhs) const { return compareTo(rhs) > 0; }
    unsigned char operator<=(const String& rhs) const { return compareTo(rhs) <= 0; }
    unsigned char operator>=(const String& rhs) const { return compareTo(rhs) >= 0; }
    // unsigned char equalsIgnoreCase(const String& s) const;
    // unsigned char equalsConstantTime(const String& s) const;
    unsigned char startsWith(const String& prefix) const {
        auto pref = backbuf.substr(0, prefix.backbuf.size());
        return pref == prefix.backbuf;
    }
    unsigned char startsWith(const String& prefix, unsigned int offset) const {
        if (prefix.length() > backbuf.size()) {
            return false;
        }
        auto pref = backbuf.substr(offset, prefix.backbuf.size() - offset);
        return pref == prefix.backbuf;
    }
    unsigned char endsWith(const String& suffix) const {
        if (suffix.length() > backbuf.size()) {
            return false;
        }
        auto suff = backbuf.substr(backbuf.size() - suffix.backbuf.size());
        return suff == suffix.backbuf;
    }

    // character access
    char  charAt(unsigned int index) const { return backbuf[index]; }
    void  setCharAt(unsigned int index, char c) { backbuf[index] = c; }
    char  operator[](unsigned int index) const { return backbuf[index]; }
    char& operator[](unsigned int index) { return (&*backbuf.begin())[index]; }
    void  getBytes(unsigned char* buf, unsigned int bufsize, unsigned int index = 0) const {
        for (unsigned int i = 0; i < bufsize; ++i) {
            buf[i] = backbuf[i + index];
        }
    }
    void toCharArray(char* buf, unsigned int bufsize, unsigned int index = 0) const { getBytes((unsigned char*)buf, bufsize, index); }
    const char* c_str() const { return backbuf.c_str(); }
    char*       begin() { return &*backbuf.begin(); }
    char*       end() { return &*backbuf.end(); }
    const char* begin() const { return &*backbuf.begin(); }
    const char* end() const { return &*backbuf.end(); }

    // search
    int indexOf(char ch) const { return int(std::find(backbuf.begin(), backbuf.end(), ch) - backbuf.begin()); }
    int indexOf(char ch, unsigned int fromIndex) const {
        return int(std::find(backbuf.begin() + fromIndex, backbuf.end(), ch) - backbuf.begin());
    }

    // int    indexOf(const String& str) const;
    // int    indexOf(const String& str, unsigned int fromIndex) const;
    // int    lastIndexOf(char ch) const;
    // int    lastIndexOf(char ch, unsigned int fromIndex) const;
    // int    lastIndexOf(const String& str) const;
    // int    lastIndexOf(const String& str, unsigned int fromIndex) const;

    String substring(int index, int length) { return String(backbuf.substr(index, length)); }
    String substring(int index) { return String(backbuf.substr(index)); }

    // modification
    // void replace(char find, char replace);
    // void replace(const String& find, const String& replace);
    // void remove(unsigned int index);
    // void remove(unsigned int index, unsigned int count);
    // void toLowerCase(void);
    // void toUpperCase(void);
    // void trim(void);

    // parsing/conversion
    long   toInt() const { return atoi(backbuf.c_str()); }
    float  toFloat() const { return std::stof(backbuf.c_str()); }
    double toDouble() const { return std::stod(backbuf.c_str()); }
};

class StringSumHelper : public String {
public:
    StringSumHelper(const String& s) :
        String(s) {
    }
    StringSumHelper(const char* p) :
        String(p) {
    }
    StringSumHelper(char c) :
        String(c) {
    }
    StringSumHelper(unsigned char num) :
        String(num) {
    }
    StringSumHelper(int num) :
        String(num) {
    }
    StringSumHelper(unsigned int num) :
        String(num) {
    }
    StringSumHelper(long num) :
        String(num) {
    }
    StringSumHelper(unsigned long num) :
        String(num) {
    }
    StringSumHelper(float num) :
        String(num) {
    }
    StringSumHelper(double num) :
        String(num) {
    }
};

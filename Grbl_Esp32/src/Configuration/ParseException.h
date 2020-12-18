#pragma once

namespace Configuration {
    class ParseException {
        int         line_;
        int         column_;
        const char* description_;

    public:
        ParseException()                      = default;
        ParseException(const ParseException&) = default;

        ParseException(const char* start, const char* current, const char* description) : description_(description) {
            line_   = 1;
            column_ = 1;
            while (start != current) {
                if (*start == '\n') {
                    ++line_;
                    column_ = 1;
                }
                ++start;
            }
        }

        inline int         LineNumber() const { return line_; }
        inline int         ColumnNumber() const { return column_; }
        inline const char* What() const { return description_; }
    };
}

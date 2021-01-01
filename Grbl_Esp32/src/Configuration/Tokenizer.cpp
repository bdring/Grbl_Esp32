#include "Tokenizer.h"

#include "ParseException.h"
#include <cstdlib>

namespace Configuration {

    Tokenizer::Tokenizer(const char* start, const char* end) : start_(start), current_(start), end_(end), token_() {
        // If start is a yaml document start ('---' [newline]), skip that first.
        if (EqualsCaseInsensitive("---")) {
            for (int i = 0; i < 3; ++i) {
                Inc();
            }
            while (IsWhiteSpace()) {
                Inc();
            }
            while (Current() == '\r' || Current() == '\n') {
                Inc();
            }

            start_ = current_;
        }
    }

    void Tokenizer::ParseError(const char* description) const { throw ParseException(start_, current_, description); }

    void Tokenizer::Tokenize() {
        // We parse 1 line at a time. Each time we get here, we can assume that the cursor
        // is at the start of the line.

    parseAgain:
        int indent = 0;

        while (!Eof() && IsSpace()) {
            Inc();
            ++indent;
        }

        if (!Eof()) {
            switch (Current()) {
            case '\t':
                // TODO FIXME: We can do tabs or spaces, not both. However, we *could* let the user decide.
                ParseError("Indentation through tabs is not allowed. Convert all tabs to spaces please.");
                break;

            case '#':  // Comment till end of line
                Inc();
                while (!Eof() && !IsEndLine()) {
                    Inc();
                }
                return;

            case '\r':
            case '\n':
                Inc();
                if (!Eof() && Current() == '\n') {
                    Inc();
                }  // \r\n
                goto parseAgain;

            default:
                if (!IsAlpha()) {
                    ParseError("Expected identifier.");
                }

                token_.keyStart_ = current_;
                Inc();
                while (!Eof() && (IsAlpha() || IsDigit() || Current() == '_')) {
                    Inc();
                }
                token_.keyEnd_ = current_;

                // Skip whitespaces:
                while (!Eof() && IsWhiteSpace()) {
                    Inc();
                }

                if (Current() != ':') {
                    ParseError("After a key or section name, we expect a colon character ':'.");
                }
                Inc();

                // Skip whitespaces after the colon:
                while (!Eof() && IsWhiteSpace()) {
                    Inc();
                }

                token_.indent_ = indent;
                if (IsEndLine()) {
                    token_.kind_ = TokenKind::Section;

                    Inc();
                    if (!Eof() && Current() == '\n') {
                        Inc();
                    }  // \r\n
                }
                else {
                    switch (Current()) {
                    case '"':
                    case '\'': {
                        auto delimiter = Current();

                        token_.kind_ = TokenKind::String;
                        Inc();
                        token_.sValueStart_ = current_;
                        while (!Eof() && Current() != delimiter && !IsEndLine()) {
                            Inc();
                        }
                        token_.sValueEnd_ = current_;
                        if (Current() != delimiter) {
                            ParseError("Could not find matching delimiter in string value.");
                        }
                        Inc();
                    } break;

                    default:
                        if (EqualsCaseInsensitive("true")) {
                            token_.kind_ = TokenKind::Boolean;
                            token_.bValue_ = true;

                            for (auto i = 0; i < 4; ++i) {
                                Inc();
                            }
                        }
                        else if (EqualsCaseInsensitive("false")) {
                            token_.kind_ = TokenKind::Boolean;
                            token_.bValue_ = false;

                            for (auto i = 0; i < 5; ++i) {
                                Inc();
                            }
                        }
                        else if (IsDigit() || Current() == '-') {
                            auto doubleOrIntStart = current_;

                            int  intValue = 0;
                            bool negative = false;

                            if (Current() == '-') {
                                Inc();
                                negative = true;
                            }

                            while (IsDigit()) {
                                intValue = intValue * 10 + int(Current() - '0');
                                Inc();
                            }

                            if (Current() == 'e' || Current() == 'E' || Current() == '.' ||  // markers
                                (current_ - doubleOrIntStart) >= 9) {  // liberal interpretation of 'out of int range'
                                char* floatEnd;
                                token_.fValue_ = strtod(doubleOrIntStart, &floatEnd);
                                token_.kind_ = TokenKind::FloatingPoint;

                                current_ = floatEnd;
                            }
                            else {
                                if (negative) {
                                    intValue = -intValue;
                                }
                                token_.iValue_ = intValue;
                                token_.kind_ = TokenKind::IntegerValue;
                            }
                        }
                        else {
                            // If it's not 'true', not 'false', and not a digit, we have a string delimited by a whitespace
                            token_.kind_ = TokenKind::String;
                            token_.sValueStart_ = current_;
                            while (!Eof() && !IsWhiteSpace() && !IsEndLine()) {
                                Inc();
                            }
                            token_.sValueEnd_ = current_;
                        }
                        break;
                    }

                    // Skip more whitespaces
                    while (!Eof() && IsSpace()) {
                        Inc();
                    }

                    // A comment after a key-value pair is allowed.
                    if (Current() == '#') {
                        Inc();
                        while (!Eof() && !IsEndLine()) {
                            Inc();
                        }
                    }

                    // Should be EOL or EOF at this point.
                    if (!IsEndLine() && !Eof()) {
                        ParseError("Expected line end after key/value pair.");
                    }
                }
            }
        }
        else {
            token_.kind_ = TokenKind::Eof;
            token_.indent_ = 0;
        }
    }
}

/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Tokenizer.h"

#include "ParseException.h"

#include <cstdlib>

namespace Configuration {

    Tokenizer::Tokenizer(const char* start, const char* end) : current_(start), end_(end), start_(start), token_() {
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
                    Inc();
                    if (!Eof() && Current() == '\n') {
                        Inc();
                    }  // \r\n
                    goto parseAgain;
                case '\n':
                    // \n without a preceding \r
                    Inc();
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
                        log_debug("Section " << StringRange(token_.keyStart_, token_.keyEnd_).str());

                        Inc();
                        //                        if (!Eof() && Current() == '\n') {
                        //                            Inc();
                        //                        }  // \r\n
                    } else {
                        if (Current() == '"' || Current() == '\'') {
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
                            log_debug("StringQ " << StringRange(token_.keyStart_, token_.keyEnd_).str() << " "
                                                 << StringRange(token_.sValueStart_, token_.sValueEnd_).str());
                        } else {
                            token_.kind_        = TokenKind::String;
                            token_.sValueStart_ = current_;
                            while (!Eof() && !IsWhiteSpace() && !IsEndLine()) {
                                Inc();
                            }
                            token_.sValueEnd_ = current_;
                            log_debug("String " << StringRange(token_.keyStart_, token_.keyEnd_).str() << " "
                                                << StringRange(token_.sValueStart_, token_.sValueEnd_).str());
                        }
                        // Skip more whitespaces
                        while (!Eof() && IsWhiteSpace()) {
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
                    break;
            }     // switch
        } else {  // Eof()
            token_.kind_   = TokenKind::Eof;
            token_.indent_ = 0;
        }
    }
}

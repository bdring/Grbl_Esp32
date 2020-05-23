// Class for preprocessing GCode lines to convert text to uppercase
// while removing whitespace and comments.
// Conversion can be done in-place on a char array.
//
// There are two usage models - feed in characters one at a time,
// or feed in a string.  For character-at-at-time conversion:
//   auto gcpp = new GCodePreprocessor(string, maxlen);
//   while (c = getchar()) {
//        if (gcpp->next(c)) {
//             return gcpp->end() != maxlen; // True for okay
//        }
//    }
//    return gcpp->end() != 0;  // True if not empty
//
// For whole-string conversion:
//    auto gcpp = new GCodePreprocessor(outstring, maxlen);
//    error = gcpp->convertString(instring);  // True if error
// Outstring and instring can be the same, in which case maxlen
// should be set to strlen(outstring)+1;
// The only possible error for convertString() is if outstring is
// too short to receive all the converted characters from instring.
// If instring and outstring are the same string, that cannot happen
// because characters are never added, only (possibly) removed.
#pragma once
#include "grbl.h"

class GCodePreprocessor {
private:
    char* line;
    int maxlen;
    char comment[LINE_BUFFER_SIZE];
    int index;
    int line_flags;
    int comment_char_counter;
public:
    GCodePreprocessor()
    {}

    void begin(char* _line, int _maxlen);

    // Returns the length of the preprocessed string
    int end();

    bool next(char c);
    // Returns true on error
    bool convertString(const char *s);
};

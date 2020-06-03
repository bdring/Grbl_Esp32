// Class for preprocessing GCode lines to convert text to uppercase
// while removing whitespace and comments.
// Conversion can be done in-place on a char array.

#include "grbl.h"
#include "GCodePreprocessor.h"

// Returns false if the end was reached
int GCodePreprocessor::end() {
    if (index != maxlen) {
        line[index] = '\0';
    }
    return index;
}

void GCodePreprocessor::begin(char* _line, int _maxlen) {
    line = _line;
    maxlen = _maxlen;
    index = 0;
    line_flags = 0;
    comment_char_counter = 0;
}

// Returns true when a newline is found or no more characters
// can be added to the string.
bool GCodePreprocessor::next(char c) {
    if (line_flags & LINE_FLAG_COMMENT_PARENTHESES)    // capture all characters into a comment buffer
        comment[comment_char_counter++] = c;
    if (c == '\r' || c == ' ') {
        // ignore these whitespace items
    } else if (c == '(') {
        line_flags |= LINE_FLAG_COMMENT_PARENTHESES;
        comment_char_counter = 0;
    } else if (c == ')') {
        // End of '()' comment. Resume line allowed.
        if (line_flags & LINE_FLAG_COMMENT_PARENTHESES) {
            line_flags &= ~(LINE_FLAG_COMMENT_PARENTHESES);
            comment[comment_char_counter] = '\0'; // null terminate
            report_gcode_comment(comment);
        }
    } else if (c == ';') {
        // NOTE: ';' comment to EOL is a LinuxCNC definition. Not NIST.
        if (!(line_flags & LINE_FLAG_COMMENT_PARENTHESES))   // semi colon inside parentheses do not mean anything
            line_flags |= LINE_FLAG_COMMENT_SEMICOLON;
    } else if (c == '%') {
        // discard this character
    } else if (c == '\n') { // found the newline, so mark the end and return 1
        return true;
    } else { // add characters to the line
        if (!line_flags) {
            c = toupper(c); // make upper case
            line[index] = c;
            index++;
        }
    }
    if (index == maxlen) { // name is too long so return false
        report_status_message(STATUS_OVERFLOW, SD_client);
        return true;
    }
    return false;
}

// Returns true on error
bool GCodePreprocessor::convertString(const char* s) {
    for ( ; *s; s++) {
        if (next(*s)) {
            return end() == maxlen;
        }
    }
    // If we wanted to return false for a zero-length string,
    // we could say:  return end() != 0;
    (void)end();
    return false;
}

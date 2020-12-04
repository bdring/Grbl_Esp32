// Class for creating JSON-encoded strings.

#include "../Grbl.h"

#include "JSONEncoder.h"

namespace WebUI {
    // Constructor that supplies a default falue for "pretty"
    JSONencoder::JSONencoder() : JSONencoder(false) {}

    // Constructor.  If _pretty is true, newlines are
    // inserted into the JSON string for easy reading.
    JSONencoder::JSONencoder(bool pretty) : pretty(pretty), level(0), str("") { count[level] = 0; }

    // Private function to add commas between
    // elements as needed, omitting the comma
    // before the first element in a list.
    // If pretty-printing is enabled, a newline
    // is added after the comma.
    void JSONencoder::comma_line() {
        if (count[level]) {
            add(',');
            line();
        }
        count[level]++;
    }

    // Private function to add commas between
    // elements as needed, omitting the comma
    // before the first element in a list.
    void JSONencoder::comma() {
        if (count[level]) {
            add(',');
        }
        count[level]++;
    }

    // Private function to add a name enclosed with quotes.
    void JSONencoder::quoted(const char* s) {
        add('"');
        str.concat(s);
        add('"');
    }

    // Private function to increment the nesting level.
    // It's necessary to account for the level in order
    // to handle commas properly, as each level must
    // know when to omit the comma.
    void JSONencoder::inc_level() {
        if (++level == MAX_JSON_LEVEL) {
            --level;
        }
        count[level] = 0;
    }

    // Private function to increment the nesting level.
    void JSONencoder::dec_level() { --level; }

    // Private function to implement pretty-printing
    void JSONencoder::line() {
        if (pretty) {
            add('\n');
            for (int i = 0; i < 2 * level; i++) {
                add(' ');
            }
        }
    }

    // Begins the JSON encoding process, creating an unnamed object
    void JSONencoder::begin() { begin_object(); }

    // Finishes the JSON encoding process, closing the unnamed object
    // and returning the encoded string
    String JSONencoder::end() {
        end_object();
        return str;
    }

    // Starts a member element.
    void JSONencoder::begin_member(const char* tag) {
        comma_line();
        quoted(tag);
        add(':');
    }

    // Starts an array with "tag":[
    void JSONencoder::begin_array(const char* tag) {
        begin_member(tag);
        add('[');
        inc_level();
        line();
    }

    // Ends an array with ]
    void JSONencoder::end_array() {
        dec_level();
        line();
        add(']');
    }

    // Starts an object with {.
    // If you need a named object you must call begin_member() first.
    void JSONencoder::begin_object() {
        comma_line();
        add('{');
        inc_level();
    }

    // Ends an object with }.
    void JSONencoder::end_object() {
        dec_level();
        if (count[level + 1] > 1) {
            line();
        }
        add('}');
    }

    // Creates a "tag":"value" member from a C-style string
    void JSONencoder::member(const char* tag, const char* value) {
        begin_member(tag);
        quoted(value);
    }

    // Creates a "tag":"value" member from an Arduino string
    void JSONencoder::member(const char* tag, String value) {
        begin_member(tag);
        quoted(value.c_str());
    }

    // Creates a "tag":"value" member from an integer
    void JSONencoder::member(const char* tag, int value) { member(tag, String(value)); }

    // Creates an Esp32_WebUI configuration item specification from
    // a value passed in as a C-style string.
    void JSONencoder::begin_webui(const char* p, const char* help, const char* type, const char* val) {
        begin_object();
        member("F", "network");
        member("P", p);
        member("H", help);
        member("T", type);
        member("V", val);
    }

    // Creates an Esp32_WebUI configuration item specification from
    // an integer value.
    void JSONencoder::begin_webui(const char* p, const char* help, const char* type, int val) {
        begin_webui(p, help, type, String(val).c_str());
    }

    // Creates an Esp32_WebUI configuration item specification from
    // a C-style string value, with additional min and max arguments.
    void JSONencoder::begin_webui(const char* p, const char* help, const char* type, const char* val, int min, int max) {
        begin_webui(p, help, type, val);
        member("S", max);
        member("M", min);
    }
}

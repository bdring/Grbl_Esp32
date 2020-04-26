namespace JSON
{

bool addNewlines;
std::string str;

#define MAX_JSON_LEVEL 16
static char first[MAX_JSON_LEVEL];
static int level = 0;

static void add(char c)
{
    str.append(1, c);
}
void line()
{
    if (addNewlines) {
        add('\n');
    }
}
static void comma_line() {
    if (!first[level]) {
        add(',');
        line();
        first[level] = false;
    }
}
static void comma() {
    if (!first[level]) {
        add(',');
        first[level] = false;
    }
}

void quoted(const char *s)
{
    add('"');
    str.append(s);
    add('"');
}

void begin_member(const char *tag)
{
    comma();
    quoted(tag);
    add(':');
}

void end_array()
{
    add(']');
    --level;
}
void start_object()
{
    add('{');
}
void end_object()
{
    add('}');
    --level;
}
void begin_array(const char *tag)
{
    begin_member(tag);
    add('[');
    first[++level] = true;
}
void begin_object()
{
    comma_line();
    add('{');
    first[++level] = true;
}

void begin(bool addNL)
{
    begin_object();
}

std::string end()
{
    end_object();
    line();
    return str;
}

void member(const char *tag, const char *value)
{
    begin_member(tag);
    quoted(value);
}
void member(const char *tag, string value)
{
    begin_member(tag);
    quoted(value.c_str());
}
void member(const char *tag, int value)
{
    member(tag, to_string(value));
}

void minmax(int min, int max) {
    member("S", min);
    member("M", max);
}


// webui parses the JSON to create an object that drives a table
// This command is sent back later
// command: [ESP401]P=p T=type V=  (I guess the value is added later)
// Min .. Max:
// If M,S  M .. S
//  B => -127 .. 255
//  S => 0 .. 255
//  A => 7 .. 15  (0.0.0.0 .. 255.255.255.255)
//  I => 0 .. 2^31-1
// Options "O":[ { "display", "id" } ... ]
// P: pos
// T: type
// M: min_val
// S: max_val
// O: options
// command: cmd
// V: defaultvalue
// H: label
// F: F       ("network", used for filtering)
//  : vindex  autoincremented

void begin_webui(const char *p, const char *help, const char *type, const char *val)
{
    member("F", "network");
    member("P", p);
    member("H", help);
    member("T", type);
    member("V", val);
}

#if 0

void webui_member(const char *p, const char *help, const char *value, int min, int max)
{
    begin_webui(p, help, "S", value);
    minmax(min, max);
    end_object();
}

void prefs_string(const char *p, const char *help, const char *default, int min, int max)
{
    const char *value = prefs.getString(p, default);
    webui_member(p, help, value, min, max);
}

void password(const char *p, const char *help)
{
    webui_member(p, help, HIDDEN_PASSWORD, MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH);
}

void IP(const char *p, const char *help, int default)
{
    const char * value = wifi_config.IP_string_from_int(prefs.getInt(p, default)).c_str();
    begin_webui(p, help, "A", value);
    end_object();
    line();
}

void options(options_t *options)
{
    begin_array("O");
    while (options->name != NULL) {
        begin_object();
        quoted_int(options->name, options->value);
        end_object();
        options++;
    }
    end_array();
}

void webui_byte(const char *p, const char *help, char default, options_t *options)
{
    begin_webui(p, help, "B", to_string(get_prefs_char(p, default)).c_str());
    options(options);
    end_object();
    line();
}

void webui_int(const char *p, const char *help, int val, int min, int max)
{
    begin_webui(p, help, "I", to_string(val).c_str());
    minmax(min, max);
    end_object();
    line();
}
#endif
} // namespace JSON


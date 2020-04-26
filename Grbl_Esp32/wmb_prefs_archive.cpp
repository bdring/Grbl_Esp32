uint32_t nvs_handle;
// do_commit() is called with the result from an nvs_set_*()
// If that succeeded and autocommit is enabled, it tries
// to commit the setting to NVRAM.  If anything fails,
// the error string is set and true is returned; otherwise
// false is returned.
bool do_commit(esp_err_t err)
{
    if (err || (autocommit && nvs_commit(nvs_handle))) {
        set_error("Set failed!");
        return true;
    }
    return false;
}

const char *get_prefs_string(const char *key, const char *defval)
{
    static char buf[MAXSTRING];
    size_t len = 0;
    esp_err_t err = nvs_get_str(nvs_handle, key, NULL, &len);
    if (err || len > MAXLEN) {
        return defval;
    }
    err = nvs_get_str(nvs_handle, key, buf, &len);
    return err ? defval : buf;
}
bool set_prefs_string(const char *key, const char *value)
{
    return do_commit(nvs_set_str(nvs_handle, key, value));
}

int32_t get_prefs_char(const char *key, int8_t defval)
{
    int8_t value;
    esp_err_t err = nvs_get_i8(nvs_handle, key, &value);
    return err ? defval : value;
}

int32_t set_prefs_char(const char *key, int8_t value)
{
    return do_commit(nvs_set_i8(nvs_handle, key, value));
}

int32_t get_prefs_int(const char *key, int32_t defval)
{
    int32_t value;
    esp_err_t err = nvs_get_i32(nvs_handle, key, &value);
    return err ? defval : value;
}

bool set_prefs_int(const char *name, int32_t value)
{
    return do_commit(nvs_set_i32(nvs_handle, key, value));
}
char *get_prefs_IP(const char *name, const char *default)
{
    int32_t IP = get_prefs_int(name, default);
    return wifi_config.IP_string_from_int(IP).c_str();
}

bool *set_prefs_IP(const char *name, String value)
{
    if (!WiFiConfig::isValidIP(value.c_str())) {
        set_error("Incorrect IP!");
        return true;
    }
    return set_prefs_int(name, wifi_config.IP_int_from_string(parameter));
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

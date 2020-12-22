// Simple regular expression matcher from Rob Pike per
// https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html

//    c    matches any literal character c
//    .    matches any single character
//    ^    matches the beginning of the input string
//    $    matches the end of the input string
//    *    matches zero or more occurrences of the previous character

// The code therein has been reformatted into the style used in
// this project, with ints used as flags replaced by bools

static bool matchHere(const char* regexp, const char* text);

// matchStar - search for c*regexp at beginning of text
static bool matchStar(int c, const char* regexp, const char* text) {
    do {
        if (matchHere(regexp, text)) {
            return true;
        }
    } while (*text != '\0' && (*text++ == c || c == '.'));
    return false;
}

// matchHere - search for regex at beginning of text
static bool matchHere(const char* regexp, const char* text) {
    if (regexp[0] == '\0') {
        return true;
    }
    if (regexp[1] == '*') {
        return matchStar(regexp[0], regexp + 2, text);
    }
    if (regexp[0] == '$' && regexp[1] == '\0') {
        return *text == '\0';
    }
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text)) {
        return matchHere(++regexp, ++text);
    }
    return false;
}

// match - search for regular expression anywhere in text
bool regexMatch(const char* regexp, const char* text) {
    if (regexp[0] == '^') {
        return matchHere(++regexp, text);
    }
    do {
        if (matchHere(regexp, text)) {
            return true;
        }
    } while (*text++ != '\0');
    return false;
}

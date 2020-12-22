// Simple regular expression matcher from Rob Pike per
// https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html

//    c    matches any literal character c
//    .    matches any single character
//    ^    matches the beginning of the input string
//    $    matches the end of the input string
//    *    matches zero or more occurrences of the previous character

// Returns true if text contains the regular expression regexp
bool regexMatch(const char* regexp, const char* text);

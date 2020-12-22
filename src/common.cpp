#include "common.h"

u32 count_lines(std::string& s) {
    int newlines = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == '\n') {
            newlines++;
        }
    }
    return newlines;
}
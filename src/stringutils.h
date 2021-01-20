#pragma once
#include <string>
#include "common.h"

extern u32 hamming_distance(std::string s1, std::string s2){
    u32 i = 0, count = 0;
    while (s1[i] != '\0')
    {
        if (s1[i] != s2[i])
            count++;
        i++;
    }
    return count;
}

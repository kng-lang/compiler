/*
James Clarke - 2021
*/

#pragma once
#include <cstdint>

#include <spdlog/spdlog.h>

#define KNG_VERSION "a_0_2"
#define kng_log(...) {spdlog::info(__VA_ARGS__);}
#define kng_warn(...) {spdlog::warn(__VA_ARGS__);}
#define kng_errr(...) {spdlog::error(__VA_ARGS__);}

#define kng_assert(condition, ...){    \
		if (!condition) {					  \
			spdlog::critical(__VA_ARGS__);	  \
			exit(0);						  \
		}									  \
}											  			

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

u32 count_lines(std::string& s);
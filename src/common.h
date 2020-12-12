#pragma once
#include <cstdint>

#include <spdlog/spdlog.h>

#define log(...) {spdlog::info(__VA_ARGS__);}
#define warn(...) {spdlog::warn(__VA_ARGS__);}

#define assert_crash(condition, ...){    \
		if (!condition) {					  \
			spdlog::critical(__VA_ARGS__);	  \
			exit(0);						  \
		}									  \
}											  			

#define assert_warning(condition, ...){  \
		if (!condition)					      \
			spdlog::warn(__VA_ARGS__);	      \
}			

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t s32;
typedef float f32;
typedef double f64;
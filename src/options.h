#pragma once

#include "common.h"

#define EMIT_TOKEN_DEBUG 0x1<<0
#define EMIT_AST_DEBUG   0x1<<1

struct CompileOptions{
	enum CompileTarget {
		X86,
		X64,
		CLR,
	};


	CompileTarget compile_target;
	// which debug information should be emitted during compilation
	u8 debug_emission_flags = 0;
	// by default only show critical errors
	u8 error_level = 3;				
	std::string output_filename;
};
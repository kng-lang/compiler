#pragma once

#include "options.h"

// cmd line arguments usage
// kngc -emit_tokens -emit_ast -x86

struct Compiler {
	CompileOptions options;


	void compile();
};
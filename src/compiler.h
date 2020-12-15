#pragma once

#include "error.h"
#include "options.h"

// cmd line arguments usage
// kngc -emit_tokens -emit_ast -x86

struct CompileFile {
	std::string filename;
	std::string file_contents;
};

struct Compiler {
	CompileFile compile_file;
	CompileOptions options;
	ErrorHandler error_handler;

	std::string lexer_debug_output;
	std::string parser_debug_output;



	void compile(CompileFile compile_file, CompileOptions options, ErrorHandler error_handler);
};
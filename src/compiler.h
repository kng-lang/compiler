#pragma once

#include <map>
#include "common.h"
#include "error.h"

// cmd line arguments usage
// kngc -emit_tokens -emit_ast -x86


#define EMIT_TOKEN_DEBUG 0x1<<0
#define EMIT_AST_DEBUG   0x1<<1

struct CompileOptions {
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

struct TokenList;
struct AST;

// a file to be compiled
struct CompileFile {
	std::string file_path;
	std::string file_contents;
};

struct CompilationUnit;

struct Importer {
	u8 valid_import_path(std::string& path);
	u8 valid_include_path(std::string& path);

	u8 import(std::string& path);
	u8 include(std::string& path);

	// the importer manages the compilation units
	std::map<std::string, CompilationUnit> units;

};

struct Compiler {
	CompileOptions options;
	Importer importer;

	std::string lexer_debug_output;
	std::string parser_debug_output;

	void compile(CompileFile compile_file, CompileOptions options);
};

struct Importer;

struct CompilationUnit {
	Compiler* compiler;
	Importer* importer;
	CompileOptions compile_options;
	CompileFile compile_file;
	ErrorHandler error_handler;

	CompilationUnit(){}
	CompilationUnit(CompileFile compile_file, Compiler* compiler) 
		: compile_file(compile_file), compiler(compiler), importer(&compiler->importer), compile_options(compiler->options){}

	u8 compile();
	TokenList& compile_to_tokens();
	std::shared_ptr<AST> compile_to_ast();
};
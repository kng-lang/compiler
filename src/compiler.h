/*
James Clarke - 2021
*/

#pragma once

#include <memory>
#include <map>
#include "common.h"
#include "error.h"


/*

ways to handle #include

1. literally paste into the AST during parsing, we pass the current symtable to the parser

2. during directives processing, when we encounter a #include, we tell the includer to include that file later and pass it a copy of the symtable at that moment in time


*/

// cmd line arguments usage
// kngc -emit_tokens -emit_ast -x86


#define EMIT_TOKEN_DEBUG 0x1<<0
#define EMIT_AST_DEBUG   0x1<<1
#define EMIT_IR_DEBUG    0x1<<2

struct CompileOptions {
	enum CompileTarget {
		X86,
		X64,
		CLR,
	};

	enum class BuildTarget {
		ASSEMBLY,
		OBJECT,
		EXEC
	};

	CompileTarget compile_target;
	BuildTarget build_target;
	// which debug information should be emitted during compilation
	u8 debug_emission_flags = 0;
	u8 optimise = 0;
	// by default only show critical errors
	u8 error_level = 3;
	// number of threads to use during compilation (1 is the default)
	u8 threads = 1;
	// the target output
	std::string output_filename;
};

struct TokenList;
struct AST;

// a file to be compiled
struct CompileFile {
	std::string file_path;
	std::string file_contents;

	CompileFile(){}
	CompileFile(std::string& path);
};

struct CompilationUnit;

struct Importer {

	enum class DepStatus {
		OK = 0,
		CYCLIC_DEP = 1
	};

	Compiler* compiler;
	// the importer manages the compilation units
	std::map<std::string, std::shared_ptr<CompilationUnit>> units;
	std::map<std::string, std::vector<std::string>> unit_dependencies;
	u32 n_units = 0;
	u32 n_lines = 0;

	Importer(){}
	Importer(Compiler* compiler) : compiler(compiler){}
	u8 valid_import_path(std::string& path);
	DepStatus valid_include_path(std::string& current_path, std::string& path);
	u8 already_included(std::string& current_path, std::string& path);
	std::shared_ptr<CompilationUnit> new_unit(std::string& path, Compiler* compiler);
	std::shared_ptr<CompilationUnit> import(std::string& path);
	std::shared_ptr<CompilationUnit> include(std::string& current_path, std::string& path);
};

struct Compiler {
	CompileOptions options;
	Importer importer;

	std::string lexer_debug_output;
	std::string parser_debug_output;

	void compile(std::string& path, CompileOptions options);
};

struct Importer;

struct CompilationUnit {
	Compiler* compiler;
	Importer* importer;
	CompileOptions compile_options;
	CompileFile compile_file;
	ErrorHandler error_handler;

	CompilationUnit(){}
	CompilationUnit(CompileFile compile_file, Compiler* compiler);
	u8 compile();
	TokenList compile_to_tokens();
	std::shared_ptr<AST> compile_to_ast();
	void compile_to_bin();
};
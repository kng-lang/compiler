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

	CompileTarget m_compile_target;
	BuildTarget m_build_target;
	// which debug information should be emitted during compilation
	u8 m_debug_flags = 0;
	u8 m_optimise = 0;
	// by default only show critical errors
	u8 m_error_level = 3;
	// number of threads to use during compilation (1 is the default)
	u8 m_threads = 1;
	// the target output
	std::string m_output_filename;
};

struct TokenList;
struct AST;

// a file to be compiled
struct CompileFile {
	enum class FileType {
		FILE,
		RAW  // supporting compile directly in comand line or repl
	};
	std::string m_file_path;
	std::string m_file_contents;
	FileType m_file_type;

	CompileFile(){}
	CompileFile(std::string& path);
};

struct CompilationUnit;

struct Importer {

	enum class DepStatus {
		NO = 0,
		LOCAL = 1,
		LIB = 2,
		CYCLIC_DEP = 3
	};

	Compiler* m_compiler;
	// the importer manages the compilation units
	std::map<std::string, std::shared_ptr<CompilationUnit>> m_units;
	std::map<std::string, std::vector<std::string>> m_unit_dependencies;
	u32 m_unit_count = 0;
	u32 m_line_count = 0;

	Importer(){}
	Importer(Compiler* compiler) : m_compiler(compiler){}
	std::string create_import_path(std::string& path, Importer::DepStatus dep_status);
	u8 valid_import_path(std::string& path);
	DepStatus valid_include_path(std::string& current_path, std::string& path);
	u8 already_included(std::string& current_path, std::string& path);
	std::shared_ptr<CompilationUnit> new_unit(std::string& path, Compiler* compiler);
	std::shared_ptr<CompilationUnit> import(std::string& path);
	std::shared_ptr<CompilationUnit> include(std::string& current_path, std::string& path, DepStatus dep_status);
};

struct Compiler {
	CompileOptions m_options;
	Importer m_importer;

	std::string m_lexer_debug_output;
	std::string m_parser_debug_output;

	void compile(std::string& path, CompileOptions options);
};

struct Importer;

struct CompilationUnit {
	Compiler* m_compiler;
	Importer* m_importer;
	CompileOptions m_compile_options;
	CompileFile m_compile_file;
	ErrorHandler m_error_handler;

	CompilationUnit(){}
	CompilationUnit(CompileFile compile_file, Compiler* compiler);
	u8 compile();
	TokenList compile_to_tokens();
	std::shared_ptr<AST> compile_to_ast();
	void compile_to_bin();
};
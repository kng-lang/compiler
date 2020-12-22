
#include <fstream>

#include "compiler.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"


void Compiler::compile(CompileFile compile_file, CompileOptions options) {

	this->options = options;

	log("kng compiler v0_1");

	auto t1 = std::chrono::high_resolution_clock::now();
	log("compilation started");

	Importer importer;

	auto unit = CompilationUnit(compile_file, this);
	unit.compile();

	auto t2 = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	log("compilation finished in {} ms.", time);
}

u8 CompilationUnit::compile() {
	auto ast = compile_to_ast();
	return 1;
}

TokenList& CompilationUnit::compile_to_tokens() {
	// lexical analysis
	Lexer l(compile_file.file_contents, this);
	auto tokens = l.scan();
	if (compile_options.debug_emission_flags & EMIT_TOKEN_DEBUG)
		log("lexer debug {}:\n{}", compile_file.file_path, tokens.to_json());
	return tokens;
}

std::shared_ptr<AST> CompilationUnit::compile_to_ast() {
	auto tokens = compile_to_tokens();
	// parsing to an ast
	Parser p(tokens, this);
	auto ast = p.parse();
	if (compile_options.debug_emission_flags & EMIT_AST_DEBUG)
		log("parser debug {}:\n{}", compile_file.file_path, ast->to_json());
	return ast;
}

u8 Importer::valid_import_path(std::string& path) {
	// (for each of these check if the path contains a main.kng)

	// first check the path relative to the current file

	// then check the path relative to the kng install e.g. c:/kng/lib/examples/

	// then check the internet
	// e.g. #include "https://www.github.com/kng/lib/examples/"

	// then check absolute
	return 0;
}

u8 Importer::valid_include_path(std::string& path) {

	// first check the path relative to the current file

	// then check the path relative to the kng install e.g. c:/kng/lib/...

	// then check the internet
	// e.g. #include "https://www.github.com/kng/lib/example.kng"

	// then check absolute

	return 0;
}

u8 Importer::import(std::string& path) {
	return 1;
}

u8 Importer::include(std::string& path) {
	// only import if the file hasn't already been imported
	if (!units.count(path)) {

	}
	return 1;
}
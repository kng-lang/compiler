
#include <fstream>
#include <sstream>
#include "compiler.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"
#include "types.h"

CompileFile::CompileFile(std::string& path) {
	// open a test file
	std::ifstream f;
	f.open(path);
	std::stringstream buffer;
	buffer << f.rdbuf();
	file_contents = buffer.str();
}

void Compiler::compile(std::string& path, CompileOptions options) {

	this->options = options;

	log("kng compiler v0_1");

	auto t1 = std::chrono::high_resolution_clock::now();
	log("compilation started");

	importer = Importer(this);

	auto unit = importer.include(path, std::make_shared<SymTable>());
	unit.compile();

	auto t2 = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	log("compilation finished in {} ms.", time);
	log("compiled {} files & {} lines", importer.n_units, importer.n_lines);
}

u8 CompilationUnit::compile() {
	auto ast = compile_to_ast();
	return 1;
}

TokenList CompilationUnit::compile_to_tokens() {
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
	return 1;
}

u8 Importer::valid_include_path(std::string& path) {

	// first check the path relative to the current file

	// then check the path relative to the kng install e.g. c:/kng/lib/...

	// then check the internet
	// e.g. #include "https://www.github.com/kng/lib/example.kng"

	// then check absolute

	return 1;
}

u8 Importer::already_included(std::string& path) {
	return units.count(path)>0;
}

CompilationUnit Importer::import(std::string& path) {
	CompileFile f(path);
	CompilationUnit unit(f, compiler);
	return unit;
}

CompilationUnit Importer::include(std::string& path, std::shared_ptr<SymTable> sym_table) {
	CompileFile f(path);
	CompilationUnit unit(f, compiler);
	n_units++;
	n_lines += count_lines(f.file_contents);
	return unit;
}
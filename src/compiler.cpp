
#include <fstream>

#include "compiler.h"
#include "errhandler.h"
#include "lexer.h"
#include "parser.h"


void Compiler::compile(CompileFile compile_file, CompileOptions options) {

	this->compile_file = compile_file;
	this->options = options;

	log("kng compiler v0_1");

	ErrorHandler e(compile_file.file_contents);

	auto t1 = std::chrono::high_resolution_clock::now();
	log("compilation started");

	// lexical analysis
	Lexer l(compile_file.file_contents, &e);
	auto tokens = l.scan();
	if (options.debug_emission_flags & EMIT_TOKEN_DEBUG)
		lexer_debug_output = tokens.to_json();
	// parsing to an ast
	Parser p(tokens.tokens, &e);
	auto ast = p.parse();
	if (options.debug_emission_flags & EMIT_AST_DEBUG)
		parser_debug_output = ast->to_json();

	// code generation

	auto t2 = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	log("compilation finished in {} ms... compiled {} characters & {} lines", time, compile_file.file_contents.size(), count_lines(compile_file.file_contents));
	log("lexer debug: {}", lexer_debug_output);
	log("parser debug: {}", parser_debug_output);
}
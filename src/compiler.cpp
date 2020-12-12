
#include <fstream>

#include "compiler.h"
#include "errhandler.h"
#include "lexer.h"
#include "parser.h"


void Compiler::compile() {

	options.debug_emission_flags |= EMIT_TOKEN_DEBUG;
	options.debug_emission_flags |= EMIT_AST_DEBUG;


	log("kng compiler v0_1");

	// open a test file
	std::ifstream f;
	f.open("F:/kng/tests/test.kng");

	std::stringstream buffer;
	buffer << f.rdbuf();

	std::string contents = buffer.str();

	log(contents);

	ErrorHandler e(contents);


	auto t1 = std::chrono::high_resolution_clock::now();
	log("compilation started");

	// lexical analysis
	Lexer l(contents, &e);
	auto tokens = l.scan();
	if (options.debug_emission_flags & EMIT_TOKEN_DEBUG)
		log(tokens.to_json());

	// parsing to an ast
	Parser p(tokens.tokens, &e);
	auto ast = p.parse();
	if (options.debug_emission_flags & EMIT_AST_DEBUG)
		log(ast->to_json());

	// code generation

	auto t2 = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	log("compilation finished in {} ms", time);
}
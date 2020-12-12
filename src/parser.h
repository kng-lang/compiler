#pragma once

#include <memory>
#include <vector>
#include "common.h"
#include "ast.h"
#include "token.h"
#include "errhandler.h"
#include "types.h"

struct Parser {
	std::vector<Token> tokens;
	// current char being scanned

	u32 current = 0;
	ErrorHandler* err_handler;
	std::shared_ptr<AST> root_ast;

	Parser() : root_ast(){
	}

	Parser(std::vector<Token> tokens, ErrorHandler* err_handler) : root_ast(), tokens(tokens), err_handler(err_handler) {
		root_ast = std::make_shared<AST>();
	}

	std::shared_ptr<AST> parse();
	std::shared_ptr<AST> parse_stmt_block();
	u8 end_of_block();
	std::shared_ptr<AST> parse_stmt();
	std::shared_ptr<AST> parse_expression();
	std::shared_ptr<AST> parse_define();
	std::shared_ptr<AST> parse_quick_define();

	u8 end();
	Token consume(Token::Type type, const std::string err_msg);
	Token prev();
	Token peek();
	Token peek(u32 amount);
	Token peek_ahead();
	Token next();
	Token advance(u32 amount);
};
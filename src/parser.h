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
	std::shared_ptr<AST> parse_if();
	std::shared_ptr<AST> parse_for();
	std::shared_ptr<AST> parse_expression();
	std::shared_ptr<AST> parse_define();
	std::shared_ptr<AST> parse_quick_define();
	std::shared_ptr<AST> parse_assign();
	std::shared_ptr<AST> parse_lor(); // parse logical or
	std::shared_ptr<AST> parse_land(); // parse logical and
	std::shared_ptr<AST> parse_bor(); // parse bitwise or
	std::shared_ptr<AST> parse_band(); // parse bitwise and
	std::shared_ptr<AST> parse_eq(); // parse equality
	std::shared_ptr<AST> parse_comp(); // parse comparison
	std::shared_ptr<AST> parse_shift(); // bitwise shift
	std::shared_ptr<AST> parse_pm(); // plus minus
	std::shared_ptr<AST> parse_mdmr(); // mul div mod remain
	std::shared_ptr<AST> parse_un();
	std::shared_ptr<AST> parse_cast();
	std::shared_ptr<AST> parse_call();
	std::shared_ptr<AST> parse_single();

	u8 end();
	Token consume(Token::Type type, const std::string err_msg); 
	u8 consume(Token::Type type);
	Token prev();
	Token peek();
	Token peek(u32 amount);
	Token peek_ahead();
	Token next();
	Token advance(u32 amount);
};
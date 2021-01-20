#pragma once

#include <memory>
/*
James Clarke - 2021
*/

#include <vector>
#include "common.h"
#include "ast.h"
#include "token.h"
#include "error.h"
#include "types.h"

struct Compiler;

struct Parser : public TokenConsumer{


	enum class ParseResult {
		SUCCESS,
		FAIL,
	};

	ParseResult m_result;

	CompilationUnit* m_unit;
	std::shared_ptr<AST> m_root_ast;
	std::shared_ptr<SymTable<Type>> m_sym_table;
	u8 m_parsing_constant_assignment = 0;
	u8 m_parsing_variable_assignment = 0;
	u8 m_requiring_delimiter = 0;

	Parser();
	Parser(TokenList& tokens, CompilationUnit* unit);

	std::shared_ptr<AST> parse();
	std::shared_ptr<AST> parse_stmt_block();
	u8 end_of_block();
	u8 expecting_type();
	u8 expecting_expr();
	u8 expecting_def();
	Type parse_type();
	std::shared_ptr<AST> parse_stmt();
	std::shared_ptr<AST> parse_directive();
	std::shared_ptr<AST> parse_if();
	std::shared_ptr<AST> parse_for();
	std::shared_ptr<AST> parse_expression_stmt();
	std::shared_ptr<AST> parse_expression();
	std::shared_ptr<AST> parse_define();
	std::shared_ptr<AST> parse_assign();
	std::shared_ptr<AST> parse_pattern();
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
	std::shared_ptr<AST> parse_fn();
	std::shared_ptr<AST> parse_interface();
};
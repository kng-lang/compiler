#pragma once

#include <sstream>
#include <string>
#include "common.h"
#include "types.h"
#include "token.h"

struct AST {
	
	enum class Type{
		BASE,
		STMT,
		EXPR,
		ERR,
		PROG,
		STMT_BLOCK,
		STMT_EXPR,
		STMT_DEF,
		STMT_ASSIGN,
		STMT_INTER_ASSIGN,
		STMT_RET,
		STMT_CONT,
		STMT_BRK,
		STMT_IF,
		STMT_LOOP,
		EXPR_VAR,
		EXPR_INTER_GET,
		EXPR_BIN,
		EXPR_LIT,
	};

	virtual void debug();
	virtual std::string to_json();
	virtual Type type() { return Type::BASE; }
};


struct StatementAST : public AST {
	virtual std::string to_json();
	virtual Type type() { return Type::STMT; }

};

struct ExpressionAST : public AST {
	virtual Type type() { return Type::EXPR; }
};

struct ErrorAST : public StatementAST {
	std::string error_msg;
	virtual Type type() { return Type::ERR; }
};

struct ProgramAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
	virtual Type type() { return Type::PROG; }
};

struct StmtBlockAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_BLOCK; }
};

struct StmtExpressionAST : public StatementAST {
	std::shared_ptr<AST> expression;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_EXPR; }
};

struct StmtDefineAST : public StatementAST {
	std::string identifier;
	Type define_type;
	std::shared_ptr<AST> value;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_DEF; }
};

struct StmtAssignAST : public StatementAST {
	Token variable;
	std::shared_ptr<AST> value;

	StmtAssignAST(Token variable, std::shared_ptr<AST> value) : variable(variable), value(value){}

	virtual std::string to_json();
	virtual Type type() { return Type::STMT_ASSIGN; }
};

struct StmtInterfaceAssignAST : public StatementAST {
	std::shared_ptr<AST> interface_expr;
	Token member_identifier;
	std::shared_ptr<AST> value;

	StmtInterfaceAssignAST(std::shared_ptr<AST> interface_expr,
						   Token member_identifier,
						   std::shared_ptr<AST> value
		) : interface_expr(interface_expr), member_identifier(member_identifier), value(value) {}

	virtual std::string to_json();
	virtual Type type() { return Type::STMT_INTER_ASSIGN; }
};

struct StmtReturnAST : public StatementAST {
	std::shared_ptr<ExpressionAST> value;
	StmtReturnAST(){}
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_RET; }
};

struct StmtContinueAST: public StatementAST {
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_CONT; }
};

struct StmtBreakAST : public StatementAST {
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_BRK; }
};

struct StmtIfAST : public StatementAST {
	std::shared_ptr<AST> if_cond;
	std::shared_ptr<AST> if_stmt;
	virtual std::string to_json();
	virtual Type type() { return Type::STMT_IF; }
};

struct StmtLoopAST : public StatementAST {
	virtual Type type() { return Type::STMT_LOOP; }
};

struct ExprVarAST : public ExpressionAST {
	Token identifier;
	ExprVarAST(Token identifier) : identifier(identifier){}
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_VAR; }
};

struct ExprInterfaceGetAST : public ExpressionAST {
	std::shared_ptr<AST> value;
	Token member;
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_INTER_GET; }
};


struct ExprBinAST : public ExpressionAST {
	std::shared_ptr<AST> lhs;
	std::shared_ptr<AST> rhs;
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_BIN; }
};

// @TODO technically a function is a literal?
struct ExprLiteralAST : public ExpressionAST {
	Type literal_type;
	ExprLiteralAST(){}
	virtual std::string to_json();
	virtual Type type() { return Type::EXPR_LIT; }
};
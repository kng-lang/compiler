#pragma once

#include <sstream>
#include <string>
#include "common.h"

struct AST {
	virtual void debug();
	virtual std::string to_json();
};


struct StatementAST : public AST {
	virtual std::string to_json();
};

struct ExpressionAST : public AST {};

struct ErrorAST : public StatementAST {
	std::string error_msg;
};

struct ProgramAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
};

struct StmtBlockAST : public StatementAST {
	std::vector<std::shared_ptr<AST>> stmts;
	virtual std::string to_json();
};

struct StmtExpressionAST : public StatementAST {
	std::shared_ptr<AST> expression;
	virtual std::string to_json();
};

struct StmtDefineAST : public StatementAST {
	virtual std::string to_json();
};

struct StmtQuickDefineAST : public StatementAST {
	virtual std::string to_json();
};
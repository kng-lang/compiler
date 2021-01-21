/*
James Clarke - 2021
*/

#pragma once

#include <memory>

#include "common.h"
#include "ast.h"

struct CompilationUnit;

struct Generator : public ASTVisitor {


	enum class GeneratorResult {
		SUCCESS,
		FAIL,
	};

	GeneratorResult m_result;

	virtual void generate() = 0;
	virtual void* visit_program(ProgramAST* program_ast) = 0;
	virtual void* visit_stmt_block(StmtBlockAST* stmt_block_ast) = 0;
	virtual void* visit_stmt_expression(StmtExpressionAST* stmt_expression_ast) = 0;
	virtual void* visit_stmt_define(StmtDefineAST* stmt_define_ast) = 0;
	virtual void* visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) = 0;
	virtual void* visit_stmt_assign(StmtAssignAST* stmt_assign_ast) = 0;
	virtual void* visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast) = 0;
	virtual void* visit_stmt_return(StmtReturnAST* stmt_return_ast) = 0;
	virtual void* visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast) = 0;
	virtual void* visit_stmt_break_ast(StmtBreakAST* stmt_break_ast) = 0;
	virtual void* visit_stmt_if_ast(StmtIfAST* stmt_if_ast) = 0;
	virtual void* visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast) = 0;
	virtual void* visit_expr_fn_ast(ExprFnAST* expr_fn_ast) = 0;
	virtual void* visit_expr_cast_ast(ExprCastAST* expr_cast_ast) = 0;
	virtual void* visit_expr_var_ast(ExprVarAST* expr_var_ast) = 0;
	virtual void* visit_expr_interface_get_ast(ExprGetAST* expr_interface_get_ast) = 0;
	virtual void* visit_expr_bin_ast(ExprBinAST* expr_bin_ast) = 0;
	virtual void* visit_expr_un_ast(ExprUnAST* expr_un_ast) = 0;
	virtual void* visit_expr_group_ast(ExprGroupAST* expr_group_ast) = 0;
	virtual void* visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) = 0;
	virtual void* visit_expr_literal_array_ast(ExprLiteralArrayAST* expr_literal_array_ast) = 0;
	virtual void* visit_expr_type_ast(ExprTypeAST* expr_type_ast) = 0;

};

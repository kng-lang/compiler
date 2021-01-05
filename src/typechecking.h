/*
James Clarke - 2021
*/

#pragma once

#include <memory>

#include "types.h"
#include "common.h"
#include "ast.h"

struct CompilationUnit;

struct TypeChecker : public ASTVisitor {
	CompilationUnit* unit;
	SymTable<std::shared_ptr<Type>> sym_table;


	TypeChecker() {}
	TypeChecker(std::shared_ptr<AST> ast, CompilationUnit* unit) : ASTVisitor(ast), unit(unit) {}

	
	// cast any ast (must be an expression) to a given type
	void niave_cast_ast(Type t, std::shared_ptr<AST> ast);
	std::shared_ptr<AST> check();

	void cast_array(Type* r_type, Type l_type, std::shared_ptr<ExprLiteralArrayAST> array_ast);

	virtual void* visit_program(ProgramAST* program_ast);
	virtual void* visit_stmt_block(StmtBlockAST* stmt_block_ast);
	virtual void* visit_stmt_expression(StmtExpressionAST* stmt_expression_ast);
	virtual void* visit_stmt_define(StmtDefineAST* stmt_define_ast);
	virtual void* visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast);
	virtual void* visit_stmt_assign(StmtAssignAST* stmt_assign_ast);
	virtual void* visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast);
	virtual void* visit_stmt_return(StmtReturnAST* stmt_return_ast);
	virtual void* visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast);
	virtual void* visit_stmt_break_ast(StmtBreakAST* stmt_break_ast);
	virtual void* visit_stmt_if_ast(StmtIfAST* stmt_if_ast);
	virtual void* visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast);
	virtual void* visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast);
	virtual void* visit_expr_fn_ast(ExprFnAST* expr_fn_ast);
	virtual void* visit_expr_cast_ast(ExprCastAST* expr_cast_ast);
	virtual void* visit_expr_call_ast(ExprCallAST* expr_call_ast);
	virtual void* visit_expr_var_ast(ExprVarAST* expr_var_ast);
	virtual void* visit_expr_interface_get_ast(ExprInterfaceGetAST* expr_interface_get_ast);
	virtual void* visit_expr_bin_ast(ExprBinAST* expr_bin_ast);
	virtual void* visit_expr_un_ast(ExprUnAST* expr_un_ast);
	virtual void* visit_expr_group_ast(ExprGroupAST* expr_group_ast);
	virtual void* visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast);
	virtual void* visit_expr_literal_array_ast(ExprLiteralArrayAST* expr_literal_array_ast);
};
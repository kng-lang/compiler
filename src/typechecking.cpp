/*
James Clarke - 2021
*/

#include "typechecking.h"
#include "compiler.h"

std::shared_ptr<AST> TypeChecker::check() {
	ast->visit(this);
	return ast;
}

void* TypeChecker::visit_program(ProgramAST* program_ast) { 
	for (const auto& ast : program_ast->stmts)
		ast->visit(this);
	return NULL; 
}

void* TypeChecker::visit_stmt_block(StmtBlockAST* stmt_block_ast) {
	sym_table.enter_scope();
	for (const auto& ast : stmt_block_ast->stmts)
		ast->visit(this);
	sym_table.pop_scope();
	return NULL; 
}

void* TypeChecker::visit_stmt_expression(StmtExpressionAST* stmt_expression_ast) { 
	stmt_expression_ast->expression->visit(this);
	return NULL; 
}

void* TypeChecker::visit_stmt_define(StmtDefineAST* stmt_define_ast) {

	// @TODO this is crucial, we need to add the symbol to the symbol table before we visit its children...
	// wait do we ?   x : fn { y : x } -> that isn't valid is it???

	// first check that in this scope the variable isn't already defined
	if (sym_table.entries.size()>0 
		&& sym_table.entries[sym_table.level].count(stmt_define_ast->identifier.value)>0) {
		unit->error_handler.error("symbol already defined!",
			stmt_define_ast->identifier.index,
			stmt_define_ast->identifier.line,
			stmt_define_ast->identifier.index+stmt_define_ast->identifier.length,
			stmt_define_ast->identifier.line
			);
		return NULL;
	}


	Type l_type = stmt_define_ast->define_type;
	Type* r_type=NULL;
	// finally, visit the initialisation value
	if (stmt_define_ast->is_initialised)
		r_type=(Type*)stmt_define_ast->value->visit(this);

	if (stmt_define_ast->requires_type_inference) {
		//l_type = infer_type(stmt_define_ast->value);
		//stmt_define_ast->define_type = l_type;
		assert_crash(r_type != NULL, "r_type was NULL");
		l_type = *r_type;
		stmt_define_ast->define_type = *r_type;
	}

	if (stmt_define_ast->is_initialised) {
		if (!l_type.matches_basic(*r_type)) {
			if(r_type->can_niave_cast(l_type)) {
				r_type->cast(l_type);
			}
			else {
				unit->error_handler.error("types do not match",
					stmt_define_ast->identifier.index,
					stmt_define_ast->identifier.line,
					stmt_define_ast->identifier.index + stmt_define_ast->identifier.length,
					stmt_define_ast->identifier.line);
					return NULL;
			}
		}
	}


	if (stmt_define_ast->is_constant && !stmt_define_ast->is_initialised) {
		unit->error_handler.error("constant variable requires initialisation",
			stmt_define_ast->identifier.index,
			stmt_define_ast->identifier.line,
			stmt_define_ast->identifier.index + stmt_define_ast->identifier.length,
			stmt_define_ast->identifier.line);
		return NULL;
	}

	sym_table.add_symbol(stmt_define_ast->identifier.value,
		SymTableEntry<std::shared_ptr<Type>>(
			std::make_shared<Type>(l_type),
			stmt_define_ast->is_global,
			stmt_define_ast->is_constant));

	// if we are at the first scope then this is a global variable
	if (sym_table.level == 0) {
		stmt_define_ast->is_global = 1;
	}

	return NULL; 
}

void* TypeChecker::visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) { return NULL; }

void* TypeChecker::visit_stmt_assign(StmtAssignAST* stmt_assign_ast) {
	// @TODO support error when assigning to constant
	auto l_type = sym_table.get_symbol(stmt_assign_ast->variable.value);
	assert_crash(l_type.value != NULL, "lhs was null when type checking assignment");
	if (l_type.is_constant) {
		unit->error_handler.error("cannot re-assign to constant",
			stmt_assign_ast->variable.index,
			stmt_assign_ast->variable.line,
			stmt_assign_ast->variable.index + stmt_assign_ast->variable.length,
			stmt_assign_ast->variable.line);
	}
	auto r_type = (Type*)stmt_assign_ast->value->visit(this);
	if (!l_type.value->matches_basic(*r_type)) {

		// see if we can cast
		if (l_type.value->can_niave_cast(*r_type)) {
			r_type->cast(*l_type.value);
			return NULL;
		}

		unit->error_handler.error("lhs doesn't match rhs",
			stmt_assign_ast->variable.index,
			stmt_assign_ast->variable.line,
			stmt_assign_ast->variable.index + stmt_assign_ast->variable.length,
			stmt_assign_ast->variable.line);
	}
	return NULL; 
}

void* TypeChecker::visit_stmt_interface_assign(StmtInterfaceAssignAST* stmt_interface_assign_ast) { return NULL; }

void* TypeChecker::visit_stmt_return(StmtReturnAST* stmt_return_ast) { 
	return NULL; 
}

void* TypeChecker::visit_stmt_continue_ast(StmtContinueAST* stmt_continue_ast) { return NULL; }

void* TypeChecker::visit_stmt_break_ast(StmtBreakAST* stmt_break_ast) { return NULL; }

void* TypeChecker::visit_stmt_if_ast(StmtIfAST* stmt_if_ast) { 
	stmt_if_ast->if_cond->visit(this);
	stmt_if_ast->if_stmt->visit(this);
	if (stmt_if_ast->has_else)
		stmt_if_ast->else_stmt->visit(this);
	return NULL; 
}

void* TypeChecker::visit_stmt_loop_ast(StmtLoopAST* stmt_loop_ast) { return NULL; }


void* TypeChecker::visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast) { return NULL; }

void* TypeChecker::visit_expr_fn_ast(ExprFnAST* expr_fn_ast) {

	// if we are defining a constant then set the fn name to that constant
	// @TODO this is a terrible idea. imagine if the program was this:
	// x : s32 1
	// native_fn_call((){})
	// we just declared a constant, however the lambda has nothing to do with it?
	// we need to have a field in the ExprFnAST that says if it is a lambda or not

	// if the fn isn't a lambda (meaning it must be assigned to a constant), update its name
	if (!expr_fn_ast->is_lambda) {
		expr_fn_ast->full_type.fn_signature.anonymous_identifier = sym_table.latest_entry.first;
	}

	// @TODO if the fn is assigned to a constant, the functions name should be the same
	// as the constan'ts identifier
	if(expr_fn_ast->has_body)
		expr_fn_ast->body->visit(this);

	return (void*)&expr_fn_ast->full_type;
}


void* TypeChecker::visit_expr_cast_ast(ExprCastAST* expr_cast_ast) {
	return (void*)&expr_cast_ast->t;
}

void* TypeChecker::visit_expr_var_ast(ExprVarAST* expr_var_ast) {
	return NULL;
}

void* TypeChecker::visit_expr_interface_get_ast(ExprInterfaceGetAST* expr_interface_get_ast) { return NULL; }

void* TypeChecker::visit_expr_bin_ast(ExprBinAST* expr_bin_ast) { return NULL; }

void* TypeChecker::visit_expr_un_ast(ExprUnAST* expr_un_ast) { return NULL; }

void* TypeChecker::visit_expr_group_ast(ExprGroupAST* expr_group_ast) {
	return expr_group_ast->visit(this);
}

void* TypeChecker::visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) { 
	return (void*)&expr_literal_ast->t;
}
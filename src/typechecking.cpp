/*
James Clarke - 2021
*/

#include "typechecking.h"
#include "compiler.h"
#include "stringutils.h"


u8 TypeChecker::is_l_value(std::shared_ptr<AST> ast) { 
	return 1;
}

u8 TypeChecker::is_r_value(std::shared_ptr<AST> ast) { 
	return 1; 
}

// cast any ast (must be an expression) to a given type
void TypeChecker::niave_cast_ast(Type t, std::shared_ptr<AST> ast) {
	switch (ast->type()) {
	case AST::ASTType::EXPR_LIT: {
		auto lit = std::static_pointer_cast<ExprLiteralAST>(ast);
		lit->t.cast(t);
		return;
	}
	case AST::ASTType::EXPR_LIT_ARRAY: {
		auto arr = std::static_pointer_cast<ExprLiteralArrayAST>(ast);
		arr->array_type.m_type = t.m_type; // @TODO should we cast the array_type here?
		arr->contained_type.cast(t);
		for (const auto& value : arr->values)
			niave_cast_ast(t, value);
		return;
	}
	}
}

std::shared_ptr<AST> TypeChecker::check() {
	m_ast->visit(this);
	return m_ast;
}

void* TypeChecker::visit_program(ProgramAST* program_ast) { 
	for (const auto& ast : program_ast->stmts)
		ast->visit(this);
	return NULL; 
}

void* TypeChecker::visit_stmt_block(StmtBlockAST* stmt_block_ast) {
	m_sym_table.enter_scope();
	for (const auto& ast : stmt_block_ast->stmts)
		ast->visit(this);
	m_sym_table.pop_scope();
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
	if (m_sym_table.entries.size()>0
		&& stmt_define_ast->identifier.m_value.compare("_")!=0
		&& m_sym_table.entries[m_sym_table.level].count(stmt_define_ast->identifier)>0) {
		
		
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL,
			Error::Type::SYMBOL_ALREADY_DEFINED,
			"symbol is already defined",
			Token::Position(
				stmt_define_ast->identifier.m_index,
				stmt_define_ast->identifier.m_index + stmt_define_ast->identifier.m_length,
				stmt_define_ast->identifier.m_line,
				stmt_define_ast->identifier.m_line
			)
		);
		return NULL;
	}

	// if the definition is a constant fn, we want the fn itself to actually be defined with the identifier
	if (!(stmt_define_ast->define_type.m_is_constant && stmt_define_ast->define_type.m_type == Type::Types::FN))
		m_sym_table.add_symbol(stmt_define_ast->identifier);

	// if we require the type to be resolved from an identifier, then do that here
	// an example is x : string; the word string needs to be resolved to a concreate type
	if (stmt_define_ast->define_type.m_interface_requires_type_finding) {
		auto entry = m_sym_table.get_symbol(stmt_define_ast->define_type.m_interface_identifier);
		stmt_define_ast->define_type = *entry.type->m_type_contained;
	}

	Type l_type = stmt_define_ast->define_type;
	Type r_type;
	Type* r_type_ptr;
	// finally, visit the initialisation value
	if (stmt_define_ast->m_is_initialised) {
		stmt_define_ast->value->visit(this);
		r_type = m_checked_type;
		r_type_ptr = m_checked_type_ptr;
	}

	if (stmt_define_ast->define_type.m_type == Type::Types::TYPE) {
		// get the contained type
		stmt_define_ast->define_type.m_type_contained = r_type_ptr;
	}

	if (stmt_define_ast->m_requires_type_inference) {
		//l_type = infer_type(stmt_define_ast->value);
		//stmt_define_ast->define_type = l_type;
		kng_assert(r_type_ptr != NULL, "r_type was NULL");
		l_type = r_type;
		stmt_define_ast->define_type = r_type;
	}

	if (stmt_define_ast->m_is_initialised) {
		if (!l_type.matches(r_type)) {
			// @TODO the problem here is that if we are dealing with an array, we need to cast each element individually
			if(r_type_ptr->can_niave_cast(l_type)) {
				niave_cast_ast(l_type, stmt_define_ast->value);
			}
			else {
				m_unit->m_error_handler.error(
					Error::Level::CRITICAL,
					Error::Type::TYPE_MISMATCH,
					std::string("types do not match. got ").append(Type::debug_types[(u32)r_type.m_type]).append(", expected ").append(Type::debug_types[(u32)l_type.m_type]),
					stmt_define_ast->value->m_position
				);
			}
		}
	}


	if (stmt_define_ast->m_is_constant && !stmt_define_ast->m_is_initialised) {
		m_unit->m_error_handler.error("constant variable requires initialisation",
			stmt_define_ast->identifier.m_index,
			stmt_define_ast->identifier.m_line,
			stmt_define_ast->identifier.m_index + stmt_define_ast->identifier.m_length,
			stmt_define_ast->identifier.m_line);
		return NULL;
	}

	m_sym_table.set_symbol(stmt_define_ast->identifier,
		SymTableEntry(
			nullptr,
			&stmt_define_ast->define_type,
			stmt_define_ast->m_is_global,
			stmt_define_ast->m_is_constant));

	// if we are at the first scope then this is a global variable
	if (m_sym_table.level == 0) {
		stmt_define_ast->m_is_global = 1;
	}

	m_checked_type = stmt_define_ast->define_type;
	m_checked_type_ptr = &stmt_define_ast->define_type;

	return NULL; 
}

void* TypeChecker::visit_stmt_interface_define(StmtInterfaceDefineAST* stmt_interface_define_ast) { return NULL; }

void* TypeChecker::visit_stmt_assign(StmtAssignAST* stmt_assign_ast) {

	// @TODO support error handling here


	//// @TODO support error when assigning to constant
	//auto l_type = sym_table.get_symbol(stmt_assign_ast->variable.value);

	// first check if the left and right are lvalues
	if(!is_l_value(stmt_assign_ast->assignee))
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL, Error::Type::NOT_L_VALUE,
			"lhs is not l-value",
			stmt_assign_ast->assignee->m_position
		);
	// first check if the left and right are lvalues
	if (!is_r_value(stmt_assign_ast->value))
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL, Error::Type::NOT_R_VALUE,
			"rhs is not r-value",
			stmt_assign_ast->value->m_position
		);



	stmt_assign_ast->assignee->visit(this);
	auto l_type = m_checked_type;

	//if (l_type.is_constant) {
	//	unit->error_handler.error("cannot re-assign to constant",
	//		stmt_assign_ast->variable.index,
	//		stmt_assign_ast->variable.line,
	//		stmt_assign_ast->variable.index + stmt_assign_ast->variable.length,
	//		stmt_assign_ast->variable.line);
	//}
	stmt_assign_ast->value->visit(this);
	auto r_type = m_checked_type;
	auto r_type_ptr = m_checked_type_ptr;
	if (!l_type.matches(r_type)) {
		// see if we can cast
		if (l_type.can_niave_cast(r_type)) {
			r_type_ptr->cast(l_type);
			return NULL;
		}
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL,
			Error::Type::TYPE_MISMATCH,
			"types do not match",
			stmt_assign_ast->value->m_position
		);
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


void* TypeChecker::visit_expr_inter_ast(ExprInterfaceAST* expr_interface_ast) { 

	// if the fn isn't a lambda (meaning it must be assigned to a constant), update its name
	if (!expr_interface_ast->m_is_lambda) {
		expr_interface_ast->m_lambda_name = m_sym_table.latest_entry.first;
		expr_interface_ast->m_type.m_interface_identifier = m_sym_table.latest_entry.first;
	}

	m_sym_table.enter_scope();
	
	s32 idx = 0;
	for (const auto& def : expr_interface_ast->m_definitions) {
		def->visit(this);
		expr_interface_ast->m_type.m_interface_members.push_back(m_checked_type);
		expr_interface_ast->m_type.m_interface_member_idx[m_sym_table.latest_entry.first] = idx++;
	}
	
	m_sym_table.pop_scope();

	m_checked_type_ptr = &expr_interface_ast->m_type;
	m_checked_type = expr_interface_ast->m_type;

	return NULL; 

}

void* TypeChecker::visit_expr_fn_ast(ExprFnAST* expr_fn_ast) {

	// if we are defining a constant then set the fn name to that constant
	// @TODO this is a terrible idea. imagine if the program was this:
	// x : s32 1
	// native_fn_call((){})
	// we just declared a constant, however the lambda has nothing to do with it?
	// we need to have a field in the ExprFnAST that says if it is a lambda or not

	// if the fn isn't a lambda (meaning it must be assigned to a constant), update its name
	// !@TODO shouldn't this be the job of the code generator and not the type checker...
	if (!expr_fn_ast->is_lambda) {
		expr_fn_ast->m_lambda_name = m_sym_table.latest_entry.first;
	}

	m_sym_table.enter_scope();
	// first resolve the type of the paramaters

	std::vector<Type> operation_types;
	//!@TODO figure out param types here	
	u32 i = 0;
	for (const auto& param : expr_fn_ast->params){
		param->visit(this);
		operation_types.push_back(m_checked_type);
		i++;
	}


	// @TODO if the fn is assigned to a constant, the functions name should be the same
	// as the constan'ts identifier
	if(expr_fn_ast->has_body)
		expr_fn_ast->body->visit(this);
	
	m_sym_table.pop_scope();

	m_checked_type_ptr = &expr_fn_ast->m_type;
	m_checked_type = expr_fn_ast->m_type;

	return NULL;
	//return (void*)&expr_fn_ast->full_type;
}


void* TypeChecker::visit_expr_cast_ast(ExprCastAST* expr_cast_ast) {


	// do niave casting here on the value
	expr_cast_ast->value->visit(this);
	auto value_type = m_checked_type;
	expr_cast_ast->from_type = value_type;
	auto l_type = expr_cast_ast->from_type;
	auto r_type = expr_cast_ast->to_type;
	if (l_type.can_niave_cast(r_type)){
		m_checked_type_ptr->cast(r_type);
		expr_cast_ast->niavely_resolved = 1;
	}

	// we need to resolve the type of the from
	expr_cast_ast->from_type = infer_type(expr_cast_ast->value);
	m_checked_type = expr_cast_ast->to_type;
	return NULL;
}


void* TypeChecker::visit_expr_call_ast(ExprCallAST* expr_call_ast) {
	expr_call_ast->callee->visit(this);
	Type fn_type = m_checked_type;
	if (!(fn_type.m_type==Type::Types::FN)) {
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL,
			Error::Type::MISSING_DELIMITER,
			"callee must be a fn",
			expr_call_ast->callee->m_position
		);
		return NULL;
	}


	// @TODO // check the args
	//u32 i = 0;
	//for (const auto& arg : expr_call_ast->args) {
	//	arg->visit(this);
	//	if (m_checked_type.matches_basic(fn_type.m_fn_signature.m_operation_types.at(i + 1))) {
	//		m_unit->m_error_handler.error("fn arg does not match", 0, 0, 0, 0);
	//	}
	//	i++;
	//}

	Type return_type = fn_type.m_fn_operation_types.at(0);
	m_checked_type = return_type;
	return NULL;
}

void* TypeChecker::visit_expr_var_ast(ExprVarAST* expr_var_ast) {
	if (m_sym_table.contains_symbol(expr_var_ast->identifier)) {
		m_checked_type_ptr = (Type*)m_sym_table.get_symbol(expr_var_ast->identifier).type;
		m_checked_type = *m_checked_type_ptr;
		return NULL;
	}



	
	Token dist_token;
	u32 max_dist = _UI32_MAX;
	for (int level = m_sym_table.level; level > 0; level--) {
		auto& entries = m_sym_table.entries[level];
		for(const auto& [identifier, entry] : entries){
			auto dist = levenshtein_distance(identifier.m_value, expr_var_ast->identifier.m_value);
			if (dist < max_dist) {
				max_dist = dist;
				dist_token = identifier;
			}
		}
	}
	m_unit->m_error_handler.error(
		Error::Level::CRITICAL,
		Error::Type::MISSING_DELIMITER,
		"symbol doesn't exist",
		std::string("did you mean '").append(dist_token.m_value).append("'?"),
		expr_var_ast->m_position,
		dist_token.m_position
	);
	return NULL;
}

void* TypeChecker::visit_expr_pattern_ast(ExprPatternAST* expr_pattern_ast){
	u32 i = 0;
	std::vector<Type> pattern_types;
	// resolve the types of the pattern
	for (const auto& value : expr_pattern_ast->m_values){
		value->visit(this);
		pattern_types.push_back(m_checked_type);
		i++;
	}
	expr_pattern_ast->m_type = Type::create_pattern(pattern_types);
	m_checked_type = expr_pattern_ast->m_type;
	m_checked_type_ptr = &expr_pattern_ast->m_type;
	return NULL;
}

void* TypeChecker::visit_expr_interface_get_ast(ExprGetAST* expr_interface_get_ast) { 
	expr_interface_get_ast->m_value->visit(this);
	expr_interface_get_ast->m_interface_type = m_checked_type;
	u32 idx;
	// now try to get the index assuming the lhs is an interface
	if (expr_interface_get_ast->m_member.m_type == Token::Type::IDENTIFIER) {
		// the get is an identifier e.g. x.member
		idx = m_checked_type.m_interface_member_idx[expr_interface_get_ast->m_member];
	}
	else {
		// the get is a number e.g. x.0
		idx = stoi(expr_interface_get_ast->m_member.m_value);
	}
	expr_interface_get_ast->m_idx = idx;
	m_checked_type = expr_interface_get_ast->m_interface_type.m_interface_members.at(idx);
	m_checked_type_ptr = &expr_interface_get_ast->m_interface_type.m_interface_members.at(idx);
	return NULL;
}

void* TypeChecker::visit_expr_bin_ast(ExprBinAST* expr_bin_ast) { 
	// get the types of both sides
	expr_bin_ast->lhs->visit(this);
	Type* lhs_type_ptr = m_checked_type_ptr;
	expr_bin_ast->rhs->visit(this);
	Type* rhs_type_ptr = m_checked_type_ptr;

	if (!lhs_type_ptr->matches(*rhs_type_ptr)) {
		// see if we can cast
		if (lhs_type_ptr->can_niave_cast(*rhs_type_ptr)) {
			auto cast_result = niavely_cast_to_master_type(lhs_type_ptr, rhs_type_ptr);
			// the reason we cant do this, is because if lhs_type_ptr is a variable, then we cant just change the variable
			//*lhs_type_ptr = cast_result;
			//*rhs_type_ptr = cast_result;
			expr_bin_ast->m_value_type = cast_result.m_type;
			return NULL;
		}
		m_unit->m_error_handler.error(
			Error::Level::CRITICAL,
			Error::Type::TYPE_MISMATCH,
			"types do not match",
			expr_bin_ast->m_position
		);
	}

	//// check what operation is happening
	//switch(expr_bin_ast->op.m_type){
	//	case Token::Type::PLUS:
	//	{
	//		auto cast_result = niavely_cast_to_master_type(lhs_type_ptr, rhs_type_ptr);
	//		expr_bin_ast->m_value_type = cast_result.m_type;
	//		// attempt to niavely cast the types to the correct type
	//		if(cast_result.m_type==Type::Types::UNKNOWN){
	//			kng_log("couldn't cast the types :(");
	//		}else{
	//			kng_log("successfully casted the types in bin op!");
	//		}
	//		break;
	//	}
	//}

	return NULL; 
}

void* TypeChecker::visit_expr_un_ast(ExprUnAST* expr_un_ast) { 

	switch (expr_un_ast->op.m_type) {
	case Token::Type::TYPEOF: {
		expr_un_ast->ast->visit(this);
		expr_un_ast->m_value_type = m_checked_type;
		m_checked_type = Type::create_basic(Type::Types::TYPE);
		break;
	}
	case Token::Type::POINTER:{
		// first get the type of the group
		expr_un_ast->ast->visit(this);
		// then reduce the ptr indirection
		m_checked_type.m_ptr_indirection--;
		break;
	}
	case Token::Type::BAND: {
		// first get the type of the group
		expr_un_ast->ast->visit(this);
		// then reduce the ptr indirection
		m_checked_type.m_ptr_indirection++;
		break;
	}
	}
	return NULL; 
}

void* TypeChecker::visit_expr_group_ast(ExprGroupAST* expr_group_ast) {
	expr_group_ast->visit(this);
	return NULL;
}

void* TypeChecker::visit_expr_literal_ast(ExprLiteralAST* expr_literal_ast) { 

    m_checked_type = expr_literal_ast->t;
	m_checked_type_ptr = &expr_literal_ast->t;
	//return (void*)&expr_literal_ast->t;
	return NULL;
}

void* TypeChecker::visit_expr_literal_array_ast(ExprLiteralArrayAST* expr_literal_array_ast) {

	// figure out the type of the array literal
	Type contained_type;
	u8 type_set = 0;
	for (const auto& ast : expr_literal_array_ast->values) {
		ast->visit(this);
		Type t = m_checked_type;
		if (!type_set)
			contained_type = t;
		else {
			if (!contained_type.matches(t)) {
				kng_assert(false, "array values do not match!");
				return NULL;
			}
		}
	}

	expr_literal_array_ast->contained_type = contained_type;
	Type array_type = contained_type;
	array_type.m_is_arr = 1;
	array_type.m_arr_length = expr_literal_array_ast->size;
	expr_literal_array_ast->array_type = array_type;

	m_checked_type_ptr = &expr_literal_array_ast->array_type;
	m_checked_type = expr_literal_array_ast->array_type;
	return NULL;
	//return (void*)&expr_literal_array_ast->array_type;
}



void* TypeChecker::visit_expr_type_ast(ExprTypeAST* expr_type_ast) {
	m_checked_type = expr_type_ast->m_type;
	m_checked_type_ptr = &expr_type_ast->m_type;
	return NULL;
}

void* TypeChecker::visit_expr_included_ast(ExprIncludedAST* expr_included_ast) {
	expr_included_ast->m_ast->visit(this);
	return NULL;
}
void* TypeChecker::visit_expr_module_ast(ExprModuleAST* expr_module_ast) {
	return NULL;
}

void* TypeChecker::visit_stmt_defer_ast(StmtDeferAST* stmt_defer_ast) {
	return NULL;
}
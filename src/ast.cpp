/*
James Clarke - 2021
*/

#include "ast.h"

void AST::debug() {
	kng_log("ast");
}

std::string AST::to_json() {
	return "{AST}";
}

void* AST::visit(ASTVisitor* visitor) {
	return NULL;
}



std::string ErrorAST::to_json() {
	return "{ErrorAST}";
}

void* ErrorAST::visit(ASTVisitor* visitor) {
	return NULL;
}

std::string ProgramAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ProgramAST\",\n\"stmts\":[\n";
	for (const auto& ast : stmts)
		ss << ast->to_json() << ",\n";
	ss << "]}";
	return ss.str();
}

void* ProgramAST::visit(ASTVisitor* visitor) {
	return visitor->visit_program(this);
}

std::string StatementAST::to_json(){
	return "{StatementAST}";
}

void* StatementAST::visit(ASTVisitor* visitor) {
	return NULL;
}

std::string StmtBlockAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtBlockAST\",\n\"stmts\":[\n";
	for (const auto& ast : stmts)
		ss << ast->to_json() << ",\n";
	ss << "]}";
	return ss.str();
}

void* StmtBlockAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_block(this);
}

std::string StmtExpressionAST::to_json(){
	return "{StmtExpressionAST}";
}

void* StmtExpressionAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_expression(this);
}

std::string StmtDefineAST::to_json(){
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtDefineAST\",\n\"variable\":" << identifier.to_json() << ",\n\"type\":" << define_type.to_json() << ",\n\"value\":" << ((value!=NULL) ? value->to_json() : "\"null\"") << "}\n";
	return ss.str();
}

void* StmtDefineAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_define(this);
}

std::string StmtAssignAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtAssignAST\",\n\"assignee\":" << assignee->to_json() << ",\n\"value\":" << value->to_json() << "}\n";
	return ss.str();
}

void* StmtAssignAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_assign(this);
}

std::string StmtInterfaceAssignAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtInterfaceAssignAST\",\n\"interface\":" << variable->to_json() <<  ",\n\"member\":" << member.to_json() << ",\n\"value\":" << value->to_json() << "}\n";
	return ss.str();
}

void* StmtInterfaceAssignAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_interface_assign(this);
}


std::string StmtReturnAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtReturnAST\",\n\"value\":" << ((value != nullptr) ? value->to_json() : "null") << "\n}";
	return ss.str();
}

void* StmtReturnAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_return(this);
}


std::string StmtContinueAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtContinueAST\"\n}";
	return ss.str();
}

void* StmtContinueAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_continue_ast(this);
}

std::string StmtBreakAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtBreakAST\"\n}";
	return ss.str();
}

void* StmtBreakAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_break_ast(this);
}

std::string StmtIfAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtIfAST\",\n\"if_cond\":" << if_cond->to_json() << ",\n\"if_cond\":" << if_stmt->to_json() << "\n}";
	return ss.str();
}


void* StmtIfAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_if_ast(this);
}


std::string StmtLoopAST::to_json() {
	std::stringstream ss;
	ss << "{loop}";
	return ss.str();
}


void* StmtLoopAST::visit(ASTVisitor* visitor) {
	return visitor->visit_stmt_loop_ast(this);
}



std::string ExprInterfaceAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\":\"ExprInterfaceAST\",\n\"anonymous_name\":" << this->anonymous_name << "}\n";
	return ss.str();
}
void* ExprInterfaceAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_inter_ast(this);
}

std::string ExprFnAST::to_json(){
	std::stringstream ss;
	ss << "{\n\"type\":\"ExprFnAST\",\n\"body\":" << this->body->to_json() << "}\n";
	return ss.str();
}
void* ExprFnAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_fn_ast(this);
}


std::string ExprCastAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\":\"ExprCastAST\",\n\"value\":" << value->to_json() << ",\n\"to_type\":"<<to_type.to_json()<<"\n}\n";
	return ss.str();
}

void* ExprCastAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_cast_ast(this);
}

std::string ExprCallAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\":\"ExprCastAST\",\n\"callee\":" << callee->to_json() << ",\n\"args\":" << args->to_json() << "\n}\n";
	return ss.str();
}

void* ExprCallAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_call_ast(this);
}

std::string ExprVarAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\":\"ExprVarAST\",\n\"variable\":" << identifier.to_json() << "\n}\n";
	return ss.str();
}

void* ExprVarAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_var_ast(this);
}

std::string ExprPatternAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprPatternAST\",\nasts: [\n";
	for (const auto& ast : asts) {
		ss << ast->to_json() << ",\n";
	}
	ss << "]\n}\n";
	return ss.str();
}

void* ExprPatternAST::visit(ASTVisitor* visitor) {
	return NULL;
}

std::string ExprInterfaceGetAST::to_json() {
	return "{ExprInterfaceGetAST}";
}

void* ExprInterfaceGetAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_interface_get_ast(this);
}

std::string ExprBinAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprBinAST\",\n\"lhs\":" << lhs->to_json() << ",\n\"op\":" << op.to_json()<< ",\n\"rhs\":" << rhs->to_json() << "}\n";
	return ss.str();
}

void* ExprBinAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_bin_ast(this);
}

std::string ExprGroupAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprGroupAST\",\n\"expr\":" << expression->to_json() << "}\n";
	return ss.str();
}

void* ExprGroupAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_group_ast(this);
}

std::string ExprUnAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprUnAST\",\n\"op\":" << op.to_json() << ",\n\"ast\":" << ast->to_json() << "\n}\n";
	return ss.str();
}

void* ExprUnAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_un_ast(this);
}

std::string ExprLiteralAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprLiteralAST\",\n\"type\":" << this->t.to_json() << ",\n\"value\":" << this->v.v.as_s32 << "\n}\n";
	return ss.str();
}

void* ExprLiteralAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_literal_ast(this);
}

std::string ExprLiteralArrayAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprLiteralArrayAST\",\n\"contained_type\":" << this->contained_type.to_json() << ",\n\"size\":" << this->size << "\n}\n";
	return ss.str();
}

void* ExprLiteralArrayAST::visit(ASTVisitor* visitor) {
	return visitor->visit_expr_literal_array_ast(this);
}

void* ExpressionAST::visit(ASTVisitor* visitor)
{
	return nullptr;
}

#include "ast.h"


void AST::debug() {
	log("ast");
}

std::string AST::to_json() {
	return "{AST}";
}

std::string ProgramAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ProgramAST\",\n\"stmts\":[\n";
	for (const auto& ast : stmts)
		ss << ast->to_json() << ",\n";
	ss << "]}";
	return ss.str();
}

std::string StatementAST::to_json(){
	return "{StatementAST}";
}

std::string StmtBlockAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtBlockAST\",\n\"stmts\":[\n";
	for (const auto& ast : stmts)
		ss << ast->to_json() << ",\n";
	ss << "]}";
	return ss.str();
}

std::string StmtExpressionAST::to_json(){
	return "{StmtExpressionAST}";
}

std::string StmtDefineAST::to_json(){
	return "{StmtDefineAST}";
}

std::string StmtAssignAST::to_json() {
	return "{StmtAssignAST}";
}

std::string StmtInterfaceSetAST::to_json() {
	return "{StmtInterfaceSetAST}";
}

std::string StmtReturnAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtReturnAST\",\n\"value\":" << ((value != nullptr) ? value->to_json() : "null") << "\n}";
	return ss.str();
}

std::string StmtContinueAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtContinueAST\"\n}";
	return ss.str();
}

std::string StmtBreakAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtBreakAST\"\n}";
	return ss.str();
}

std::string StmtIfAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtIfAST\",\n\"if_cond\":" << if_cond->to_json() << ",\n\"if_cond\":" << if_stmt->to_json() << "\n}";
	return ss.str();
}










std::string ExprInterfaceGetAST::to_json() {
	return "{ExprInterfaceGetAST}";
}

std::string ExprBinAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprBinAST\",\n\"left\":" << lhs->to_json() << ",\n\"rhs\":" << rhs->to_json() << "}\n";
	return ss.str();
}


std::string ExprLiteralAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprLiteralAST\",\n\"type\":null,\n\"value\": null}\n";
	return ss.str();
}
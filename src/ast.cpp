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
	return "{}";
}

std::string StmtDefineAST::to_json(){
	return "{StmtDefineAST}";
}

std::string StmtQuickDefineAST::to_json(){
	return "{StmtQuickDefineAST}";
}

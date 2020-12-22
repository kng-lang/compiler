#include "ast.h"


void AST::debug() {
	log("ast");
}

std::string AST::to_json() {
	return "{AST}";
}

std::string ErrorAST::to_json() {
	return "{ErrorAST}";
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
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtAssignAST\",\n\"variable\":" << variable.to_json() << ",\n\"value\":" << value->to_json() << "}\n";
	return ss.str();
}

std::string StmtInterfaceAssignAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"StmtInterfaceAssignAST\",\n\"interface\":" << variable->to_json() <<  ",\n\"member\":" << member.to_json() << ",\n\"value\":" << value->to_json() << "}\n";
	return ss.str();
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








std::string ExprVarAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprVarAST\",\n\"variable\":" << identifier.to_json() << "\n}\n";
	return ss.str();
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

std::string ExprInterfaceGetAST::to_json() {
	return "{ExprInterfaceGetAST}";
}

std::string ExprBinAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprBinAST\",\n\"lhs\":" << lhs->to_json() << ",\n\"op\":" << op.to_json()<< ",\n\"rhs\":" << rhs->to_json() << "}\n";
	return ss.str();
}

std::string ExprGroupAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprGroupAST\",\n\"expr\":" << expression->to_json() << "}\n";
	return ss.str();
}

std::string ExprUnAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprUnAST\",\n\"op\":" << op.to_json() << ",\n\"ast\":" << ast->to_json() << ",\n\"side\":" << side << "}\n";
	return ss.str();
}

std::string ExprLiteralAST::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\": \"ExprLiteralAST\",\n\"type\":null,\n\"value\": null}\n";
	return ss.str();
}
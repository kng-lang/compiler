#include "types.h"
#include "ast.h"

const char* Type::debug_types[] = {
	"UNKNOWN",
	"U0",
	"U8",
	"U16",
	"U32",
	"U64",
	"S32",
	"S64",
	"F32",
	"F64",
	"CHAR",
	"STRING",
	"INTERFACE",
	"PATTERN"
};

std::string Type::to_json() {
	std::stringstream ss;
	ss << "{\n\"type\":" << debug_types[this->t] << "\n\"ref\":" << this->ref << "\n}\n";
	return ss.str();
}

Type infer_type(std::shared_ptr<AST> ast){
	switch (ast->type()) {
		case AST::ASTType::EXPR_LIT: return std::static_pointer_cast<ExprLiteralAST>(ast)->t;
		case AST::ASTType::EXPR_GROUP: return infer_type(std::static_pointer_cast<ExprGroupAST>(ast));
		default: return Type(Type::Types::UNKNOWN);
	}
}

void SymTable::add_symbol(std::string identifier, SymTableEntry entry) {
	entries[identifier] = entry;
}

std::shared_ptr<SymTable> SymTable::enter_scope(std::shared_ptr<SymTable> parent_sym_table){
	return enter_scope("block", parent_sym_table);
}

std::shared_ptr<SymTable> SymTable::enter_scope(std::string identifier, std::shared_ptr<SymTable> parent_sym_table){
	auto new_table = std::make_shared<SymTable>();
	SymTableEntry entry(std::make_shared<SymTable>(parent_sym_table));
	add_symbol(identifier, entry);
	return new_table;
}

std::shared_ptr<SymTable> SymTable::pop_scope() {
	return parent_sym_table;
}

u8 Type::matches_basic(Type other){
	return this->t == other.t
		&& this->arr == other.arr
		&& this->arr_length == other.arr_length;
}

u8 Type::matches_deep(Type other){
	return this->t == other.t
		&& this->arr == other.arr
		&& this->arr_length == other.arr_length;
		// && this->interface_signature.matches(other.interface_signature);
}
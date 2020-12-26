#include "types.h"
#include "ast.h"

const char* Type::debug_types[] = {
	"UNKNOWN",
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
	"SEQ",
	"ANY"
};

std::string Type::to_json() {
	std::stringstream ss;
	ss << "{\"type\":" << debug_types[this->t] << "\n\"ref\":" << this->ref << "\n}";
	return ss.str();
}

Type infer_type(std::shared_ptr<AST> ast){
	return Type(Type::Types::UNKNOWN);
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
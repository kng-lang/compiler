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
	"FN",
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

/*template <typename T>
void SymTable<T>::add_symbol(std::string identifier, T entry) {
	entries[level][identifier] = entry;
}

template <typename T>
T SymTable<T>::get_symbol(std::string identifier) {
	// @TODO all levels
	for (u32 i = level; i >= 0; i--) {
		if (this->entries[i].count(identifier) != 0)
			return entries[i][identifier];
	}
	return NULL;
}

template <typename T>
void SymTable<T>::enter_scope(){
	level++;
}

template <typename T>
void SymTable<T>::pop_scope() {
	level--;
}*/


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
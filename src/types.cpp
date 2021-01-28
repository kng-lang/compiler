#include "types.h"
#include "ast.h"

const char* Type::debug_types[] = {
	"UNKNOWN",
	"TYPE",
	"NAMESPACE",
	"U0",
	"U8",
	"S8",
	"U16",
	"S16",
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
	ss << "{\n\"type\":" << debug_types[(s32)this->m_type] << "\n\"ptr\":" << this->m_ptr_indirection << "\n}\n";
	return ss.str();
}

Type infer_type(std::shared_ptr<AST> ast){
	switch (ast->type()) {
		case AST::ASTType::EXPR_LIT: return std::static_pointer_cast<ExprLiteralAST>(ast)->t;
		case AST::ASTType::EXPR_GROUP: return infer_type(std::static_pointer_cast<ExprGroupAST>(ast));
		case AST::ASTType::EXPR_FN: return Type(Type::Types::FN);
		default: return Type(Type::Types::UNKNOWN);
	}
}

u8 Type::can_niave_cast(Type other) {
	if ((is_number_type() && other.is_number_type() 
		&& m_arr_length==other.m_arr_length 
		&& m_ptr_indirection==other.m_ptr_indirection)
		
		|| 
		
		// we can cast anything to a type because we take takes it's type signature
		other.m_type==Types::TYPE) {
		return 1;
	}
	return 0;
}

void Type::cast(Type other) {
	this->m_type = other.m_type;
}

u8 Type::is_number_type() {
	return is_integer_type() || is_float_type();
}

u8 Type::is_integer_type() {
	return m_type == Types::U8 || m_type == Types::S8 || m_type == Types::U16 || m_type == Types::S16 || m_type == Types::U32 || m_type == Types::S32 || m_type == Types::S64;
}

u8 Type::is_float_type() {
	return m_type == Types::F32 || m_type == Types::F64;
}

u8 Type::is_interface(){
	return m_type == Types::INTERFACE;
}
u8 Type::is_fn(){
	return m_type == Types::FN;
}

u8 Type::is_array(){
	return m_is_arr;
}

u8 Type::is_pattern(){
	return m_type == Types::PATTERN;
}
u8 Type::is_pointer(){
	return m_ptr_indirection!=0;
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


u8 Type::matches(Type other){
	if (this->m_type != other.m_type)
		return 0;
	switch (m_type) {
		case Type::Types::FN: return matches_fn(other);
		case Type::Types::INTERFACE: return matches_interface(other, Type::InterfaceMatchType::EXACT);
		case Type::Types::PATTERN: return matches_pattern(other);
		// a type is either x : type = u32; or is x : type = {...};
		// it's used to type alias a type 
		case Type::Types::TYPE: return (other.m_type == Types::TYPE || other.m_type == Types::INTERFACE);
		default:
			return
				this->m_is_arr == other.m_is_arr
				&& this->m_arr_length == other.m_arr_length
				&& this->m_ptr_indirection == other.m_ptr_indirection;
	}
}


Type niavely_cast_to_master_type(Type* lhs, Type* rhs){
	//!@TODO add error handling here
	Type highest_type = *lhs;
	// for cases where we have a f32 and an s8 for example, we need to cast the s8 to an f32
	u8 highest_type_value = (u8) lhs->m_type;
	// we can do this as the Type::Types enum is an integer
	if(((u8)rhs->m_type)>highest_type_value){
		highest_type_value=(u8)rhs->m_type;
		highest_type = *rhs;
	}
	// now cast the types to the highest type
	if(((u8)lhs->m_type)<highest_type_value){
		if (!lhs->can_niave_cast(highest_type))
			return Type(Type::Types::UNKNOWN);
		lhs->cast(highest_type);
	}else if(((u8)rhs->m_type)<highest_type_value){
		if (!rhs->can_niave_cast(highest_type))
			return Type(Type::Types::UNKNOWN);
		rhs->cast(highest_type);
	}
	return highest_type;
}



Type Type::create_basic(Type::Types t){
	return Type(t);
}

Type Type::create_pointer(Type::Types t, u32 ptr_indirection) {
	auto type = Type(t);
	type.m_ptr_indirection = ptr_indirection;
	return type;
}

Type Type::create_array(Type::Types t, u32 length) {
	auto type = Type(t);
	type.m_is_arr = 1;
	type.m_arr_length = length;
	return type;
}

Type Type::create_fn(u8 has_return, std::vector<Type> op_types) {
	auto type = Type(Type::Types::FN);
	type.m_fn_operation_types = op_types;
	type.m_fn_has_return = has_return;
	return type;
}

Type Type::create_interface(Token name, std::vector<std::pair<Token,Type>> member_types) {
	auto type = Type(Type::Types::INTERFACE);
	type.m_interface_identifier = name;
	for (int i = 0; i < member_types.size(); i++) {
		type.m_interface_member_idx[member_types[i].first] = i;
		type.m_interface_members.push_back(member_types[i].second);
	}
	return type;
}

Type Type::create_interface(Token name, std::vector<Type> member_types) {
	auto type = Type(Type::Types::INTERFACE);
	type.m_interface_identifier = name;
	type.m_interface_members = member_types;
	return type;
}

Type Type::create_interface(std::vector<Type> member_types){
	auto type = Type(Type::Types::INTERFACE);
	type.m_interface_members = member_types;
	return type;
}

Type Type::create_interface(Token name) {
	auto type = Type(Type::Types::INTERFACE);
	type.m_interface_identifier = name;
	// if the name is a builtin type we don't require type resolving
	//if(!is_builtin_type(name))
		type.m_interface_requires_type_finding = 1;
	return type;
}


u8 Type::is_builtin_type(Token& name) {
	return name.m_value.compare("type") == 0;
}

Type Type::create_pattern(std::vector<Type> types) {
	auto type = Type(Type::Types::PATTERN);
	type.m_pattern_types = types;
	return type;
}
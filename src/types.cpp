#include "types.h"
#include "ast.h"

Type u8_type() {
	return Type(Type::Types::U8);
}

Type infer_type(std::shared_ptr<AST> ast){
	return u8_type();
}
#pragma once

#include "ast.h"


/*
The job of this pass is to perform type semantic evaluation
*/
struct Pass1 {
	std::shared_ptr<SymTable> sym_table;

};
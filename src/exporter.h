/*
James Clarke - 2021
*/

#pragma once

#include "common.h"
#include "types.h"
#include "symtable.h"

#define EXPORT_DUMP_DIR "exports/" // the directory relative to kng.exe to dump the exports
#define PATH_DELIMITER "__"		   // used to delimit directories in a source file e.g. /src/main.k -> src__main.dump
#define PATH_DELIMITER_LEN 2

struct Exporter {



	u8 export_globals(SymTable sym_table);
	SymTable import_globals(std::string& path);				   // import globals into a symbol table
	void import_globals(std::string& path, SymTable* sym_table); // import globals into an existing symbol table
	std::string get_dump_name(std::string path);
	std::string reverse_dump_name(std::string path);

};
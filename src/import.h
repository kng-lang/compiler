#pragma once

#include <map>
#include "common.h"
#include "symtable.h"
#include "ast.h"


struct Compiler;
struct CompilationUnit;

struct Module {
	// these are the globals exported in this module
	SymTable m_exported_globals;
};

// an importer is soley responsible for handling importing/exporing other files
struct Importer {


	Compiler* m_compiler;
	u32 m_lines = 0;
	u32 m_units = 0;
	std::string m_lib_path;



	Importer(){}
	Importer(Compiler* compiler) : m_compiler(compiler){}

	// store a map of the imported modules
	std::map<std::string, std::shared_ptr<Module>> m_modules;
	std::map<std::string, std::shared_ptr<CompilationUnit>> m_includes;


	std::shared_ptr<Module> import_module(std::shared_ptr<CompilationUnit> unit, std::string& module_name);
	std::shared_ptr<CompilationUnit> include_file(std::string& file_name);
	std::string build_lib_path(std::string& path);
	u8 valid_import_path(std::shared_ptr<CompilationUnit> unit, std::string& path);
	u8 valid_include_path(std::string& path);
	u8 already_imported(std::shared_ptr<CompilationUnit> unit, std::string& path);
	u8 already_included(std::shared_ptr<CompilationUnit> unit, std::string& path);


	//u8 export_globals(SymTable<Type> sym_table);
	//SymTable<Type> import_globals(std::string& path);				   // import globals into a symbol table
	//void import_globals(std::string& path, SymTable<Type>* sym_table); // import globals into an existing symbol table
	//std::string get_dump_name(std::string path);
	//std::string reverse_dump_name(std::string path);

};
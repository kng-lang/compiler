#include "import.h"
#include "compiler.h"
#include <sstream>


std::shared_ptr<Module> Importer::import_module(std::shared_ptr<CompilationUnit> unit, std::string& module_name){

	// for now, we aren't caching module exports and so we are just compiling them directly






	return std::make_shared<Module>();
}

std::shared_ptr<CompilationUnit> Importer::include_file(std::string& file_name){
	CompileFile compile_file(file_name);
	auto unit = std::make_shared<CompilationUnit>(compile_file, m_compiler);
	m_units++;
	m_lines += count_lines(compile_file.m_file_contents);
	m_includes[compile_file.m_file_path] = unit;
	return unit;
}


std::string Importer::build_lib_path(std::string& path){
	std::string lib_path = std::getenv("KNG_PATH");
	std::stringstream ss;
	ss << lib_path << "lib/" << path;
	return ss.str();
}

u8 Importer::valid_import_path(std::shared_ptr<CompilationUnit> unit, std::string& path){
	// already included
	if (!m_modules.count(path))
		return 0;
	// first check the path relative to the current file
	if (FILE* file = fopen(path.c_str(), "r")) {
		fclose(file);
		return 1;
	}
	auto lib_path = build_lib_path(path);
	// first check the path relative to the current file
	if (FILE* file = fopen(lib_path.c_str(), "r")) {
		fclose(file);
		return 1;
	}
}

u8 Importer::valid_include_path(std::string& path){
	// already included
	if (m_includes.count(path)!=0)
		return 0;
	kng_log("1");
	// first check the path relative to the current file
	if (FILE* file = fopen(path.c_str(), "r")) {
		fclose(file);
		return 1;
	}
	kng_log("2");
	auto lib_path = build_lib_path(path);
	// first check the path relative to the current file
	if (FILE* file = fopen(lib_path.c_str(), "r")) {
		fclose(file);
		return 1;
	}
	kng_log("3");
	return 0;
}

u8 Importer::already_imported(std::shared_ptr<CompilationUnit> unit, std::string& path) {
	return m_modules.count(path);
}
u8 Importer::already_included(std::shared_ptr<CompilationUnit> unit, std::string& path){
	return m_includes.count(path);
}
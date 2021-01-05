/*
James Clarke - 2021
*/

#include "exporter.h"
#include "fileutils.h"

#include <filesystem>

u8 Exporter::export_globals(SymTable<Type> sym_table) {
	// exporting globals requries first serialising level 0 of the symbol table to a file 
	
	//write_file(
	//	std::filesystem::current_path()
	//		.string()
	//		.append("/")
	//		.append(EXPORT_DUMP_DIR)
	//		.append(get_dump_name("/test/file.k")).c_str(),
	//	"test data"
	//	);

	return 1;
}

SymTable<Type> Exporter::import_globals(std::string& path) {
	// the path is the path of the actual source file e.g. /src/main.k
	// this is then converted into an export dump path e.g. /kng/exports/src__main.k
	SymTable<Type> t;
	return t;
}

void Exporter::import_globals(std::string& path, SymTable<Type>* sym_table){
	auto s = import_globals(path);
}

std::string Exporter::get_dump_name(std::string path) {
	// https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
	size_t start_pos = 0;
	while ((start_pos = path.find("/", start_pos)) != std::string::npos) {
		path.replace(start_pos, 1, PATH_DELIMITER);
		start_pos += PATH_DELIMITER_LEN; // Handles case where 'to' is a substring of 'from'
	}
	return path.append(".dump");
}

std::string Exporter::reverse_dump_name(std::string path) {
	// https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
	size_t start_pos = 0;
	while ((start_pos = path.find(PATH_DELIMITER, start_pos)) != std::string::npos) {
		path.replace(start_pos, PATH_DELIMITER_LEN, "/");
		start_pos += 1; // Handles case where 'to' is a substring of 'from'
	}
	return path;
}
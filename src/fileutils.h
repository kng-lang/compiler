#pragma once

#include "common.h"
#include <fstream>

u8 write_file(const char* path, const char* data) {

	kng_log("writing file {}", path);
	std::ofstream file;
	file.open(path);
	file << data;
	file.close();
	return 0;
}
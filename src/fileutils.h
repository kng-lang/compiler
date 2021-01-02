#pragma once

#include "common.h"
#include <fstream>

u8 write_file(const char* path, const char* data) {
	std::ofstream file;
	file.open(path);
	file << data;
	file.close();
	return 0;
}
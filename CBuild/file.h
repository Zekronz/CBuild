#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include "types.h"

namespace CBuild {

	struct File {

		static void trim_path(std::string& _path);
		static void format_path(std::string& _path);
		static bool compare(const std::string& _path1, const std::string& _path2);
		static bool file_exists(const std::string _path);
		static bool directory_exists(const std::string _path);
		static bool find_files(const std::string _path, const std::string _extension, std::vector<std::filesystem::path>& _files);
		static bool read_text_file(const std::string _path, std::string& _result);
		static bool write_text_file(const std::string& _path, const std::string& _text);

	};

}


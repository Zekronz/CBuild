#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include "types.h"
#include "string_helper.h"

namespace CBuild {

	struct File {

		static void format_path(std::filesystem::path& _path);
		static bool compare(const std::filesystem::path& _path1, const std::filesystem::path& _path2);
		static bool file_exists(const std::filesystem::path& _path);
		static bool directory_exists(const std::filesystem::path& _path);
		static bool find_files(const std::filesystem::path&, const std::string _extension, std::vector<std::filesystem::path>& _files);
		static bool read_text_file(const std::filesystem::path&, std::string& _result);
		static bool write_text_file(const std::filesystem::path&, const std::string& _text);

	};

}
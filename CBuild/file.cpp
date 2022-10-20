#include "pch.h"
#include "file.h"
#include "log.h"

namespace CBuild {

	void File::trim_path(std::string& _path) {

		if (_path.length() <= 0) return;

		while (_path.length() > 0 && (_path[0] == ' ' || _path[0] == '/')) _path.erase(0, 1);
		while (_path.length() > 0 && (_path[_path.length() - 1] == ' ' || _path[_path.length() - 1] == '/')) _path.erase(_path.length() - 1, 1);

		if (_path.length() >= 2) {

			if (_path[0] == '.' && _path[1] == '/') {
				_path.erase(0, 2);
			}

		}

	}

	void File::format_path(std::filesystem::path& _path) {

		UTF8_String path_str = _path.string();

		//@TODO: Preferred separator.
		char preferred = std::filesystem::path::preferred_separator == L'\\' ? '\\' : '/';
		char other = std::filesystem::path::preferred_separator == L'\\' ? '/' : '\\';

		for (u64 i = 0; i < path_str.size(); ++i) {
			if (path_str[i] == (u32)other) path_str[i] = (u32)preferred;
		}

		if (path_str[0] == (u32)preferred) path_str.erase(0, 1);
		if (path_str[path_str.length() - 1] == (u32)preferred) path_str.erase(path_str.length() - 1, 1);

		path_str.trim();

		_path = std::filesystem::u8path(path_str.);

	}

	bool File::compare(const std::filesystem::path& _path1, const std::filesystem::path& _path2) {
		return (_path1.compare(_path2) == 0);
	}

	bool File::file_exists(const std::filesystem::path& _path) {

		if (std::filesystem::is_directory(_path)) return false;
		return std::filesystem::exists(_path);

	}

	bool File::directory_exists(const std::filesystem::path& _path) {
		return std::filesystem::is_directory(_path);
	}

	bool File::find_files(const std::filesystem::path& _path, const std::string _extension, std::vector<std::filesystem::path>& _files) {

		if (!directory_exists(_path)) return false;

		_files.clear();
		
		std::filesystem::path file;

		for (const auto& entry : std::filesystem::directory_iterator(_path)) {
			
			if (_extension.empty() || entry.path().extension() == _extension) {

				file = entry.path();
				format_path(file);

				_files.push_back(file);

			}

		}

		return true;

	}

	bool File::read_text_file(const std::filesystem::path& _path, std::string& _result) {

		std::ifstream input;
		input.open(_path, std::ios::in);

		if (!input.good()) {

			input.close();
			return false;

		}

		input.imbue(std::locale::classic());
		input.seekg(0, input.beg);

		_result.clear();

		s8 c = input.get();
		while (input.good()) {

			_result += c;
			c = input.get();

		}

		input.close();

		return true;

	}

	bool File::write_text_file(const std::filesystem::path& _path, const std::string& _text) {

		std::ofstream output;
		output.open(_path, std::ios::out);

		if (!output.good()) {

			output.close();
			return false;

		}

		output.imbue(std::locale::classic());
		output << _text;
		output.close();

		return true;

	}

}
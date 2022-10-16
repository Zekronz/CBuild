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

	void File::format_path(std::string& _path) {

		std::filesystem::path path = std::filesystem::u8path(_path);
		_path = path.string();
		
		for (u64 i = 0; i < _path.size(); ++i) {
			if (_path[i] == '\\') _path[i] = '/';
		}

		if (_path[_path.length() - 1] == '/') _path.erase(_path.length() - 1, 1);

	}

	bool File::compare(const std::string& _path1, const std::string& _path2) {
		return (std::filesystem::u8path(_path1).compare(std::filesystem::u8path(_path2)) == 0);
	}

	bool File::file_exists(const std::string _path) {

		std::filesystem::path path = std::filesystem::u8path(_path);

		if (std::filesystem::is_directory(path)) return false;
		return std::filesystem::exists(path);

	}

	bool File::directory_exists(const std::string _path) {

		std::filesystem::path path = std::filesystem::u8path(_path);
		return std::filesystem::is_directory(path);

	}

	bool File::find_files(const std::string _path, const std::string _extension, std::vector<std::filesystem::path>& _files) {

		std::filesystem::path path = std::filesystem::u8path(_path);
		if (!std::filesystem::is_directory(path)) return false;

		_files.clear();
		
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			
			if (_extension.empty() || entry.path().extension() == _extension) {
				_files.push_back(entry.path());
			}

		}

		return true;

	}

	bool File::read_text_file(const std::string _path, std::string& _result) {

		std::ifstream input;
		input.open(std::filesystem::u8path(_path), std::ios::in);

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

	bool File::write_text_file(const std::string& _path, const std::string& _text) {

		std::ofstream output;
		output.open(std::filesystem::u8path(_path), std::ios::out);

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
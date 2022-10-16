#include "pch.h"
#include "string_helper.h"

namespace CBuild {

	void String_Helper::trim(std::string& _str) {

		if (_str.length() <= 0) return;

		while (_str.length() > 0 && std::isspace(_str[0])) _str.erase(0, 1);
		while (_str.length() > 0 && std::isspace(_str[_str.length() - 1])) _str.erase(_str.length() - 1, 1);

	}

	void String_Helper::lower(std::string& _str) {

		for (u64 i = 0; i < _str.length(); ++i) {
			_str[i] = std::tolower(_str[i]);
		}

	}

}
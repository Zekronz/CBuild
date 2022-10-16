#pragma once

#include <string>

#include "types.h"

namespace CBuild {

	enum class Error_Type : uint8_t {

		None,
		Syntax_Error,
		Error,

	};

	struct Error_Handler {

		bool error_set = false;
		Error_Type error_type = Error_Type::None;
		std::string error_message = "";
		s64 error_line = 0;
		s64 error_char = 0;

		void set_error(Error_Type, const std::string _message, s64 _linePos, s64 _charPos);
		void clear_error();
		bool has_error();
		std::string get_error_message();
		void warn(const std::string _message, s64 _linePos, s64 _charPos);

	};

}
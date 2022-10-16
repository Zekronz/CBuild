#include "pch.h"
#include "error_handler.h"
#include "log.h"

namespace CBuild {

	void Error_Handler::set_error(Error_Type _type, const std::string _message, s64 _linePos, s64 _charPos) {

		error_set = true;
		error_type = _type;
		error_message = _message;
		error_line = _linePos;
		error_char = _charPos;

	}

	void Error_Handler::clear_error() {

		error_set = false;
		error_message = "";
		error_type = Error_Type::None;
		error_line = 0;
		error_char = 0;

	}

	bool Error_Handler::has_error() {
		return error_set;
	}

	std::string Error_Handler::get_error_message() {

		if (!error_set) return "";

		std::string message = "[";

		if (error_type == Error_Type::Syntax_Error) {
			message += "Syntax Error, ";
		}
		else if (error_type == Error_Type::Error) {
			message += "Error, ";
		}

		message += "Line: " + std::to_string(error_line) + ", Char: " + std::to_string(error_char) + "]\n";
		message += error_message;

		return message;

	}

	void Error_Handler::warn(const std::string _message, s64 _linePos, s64 _charPos) {
		CBUILD_WARN("[Warning, Line: {}, Char: {}]\n" + _message, _linePos, _charPos);
	}

}
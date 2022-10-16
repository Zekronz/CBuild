#pragma once

#include <string>
#include <vector>

#include "types.h"
#include "error_handler.h"

namespace CBuild {

	struct Error_Handler;

	enum class Token_Type : uint8_t {

		None,
		Command,
		String,
		Semicolon,
		End_Of_File,

	};

	struct Token {

		Token_Type type = Token_Type::None;
		std::string value = "";

		s64 line_pos = -1;
		s64 char_pos = -1;

	};

	enum class Lexer_State : uint8_t {

		Normal,
		Single_Line_Comment,
		Multi_Line_Comment,
		String,

	};

	struct Lexer {

		std::vector<Token> tokens;
		Error_Handler error_handler;

		bool is_valid_path_char(s8 _char);
		bool is_digit(s8 _char);
		bool is_letter(s8 _char);
		bool is_string_char(s8 _char);
		bool is_command(const std::string& _str);
		s8 char_at(const std::string& _source, size_t _index);
		bool parse_source(const std::string& _source);
		void print_tokens();

	};

}
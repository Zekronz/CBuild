#pragma once

#include <string>
#include <vector>

#include "types.h"
#include "error_handler.h"

namespace CBuild {

	struct Error_Handler;

	enum class Token_Type : u8 {

		None,
		String,
		Command,
		Bool,
		Semicolon,
		End_Of_File,

	};

	struct Token {

		Token_Type type = Token_Type::None;
		std::string value = "";

		s64 line_pos = -1;
		s64 char_pos = -1;

	};

	enum class Lexer_State : u8 {

		Normal,
		Single_Line_Comment,
		Multi_Line_Comment,
		String,
		Command,

	};

	struct Lexer {

		std::vector<Token> tokens;
		Error_Handler error_handler;

		bool is_valid_path_char(s8 _char);
		bool is_valid_path_string(const std::string& _path);
		bool is_valid_file_name_char(s8 _char);
		bool is_valid_file_name_string(const std::string& _path);
		bool is_digit(s8 _char);
		bool is_letter(s8 _char);
		bool is_cmd_char(s8 _char);
		s8 char_at(const std::string& _source, size_t _index);
		bool parse_source(const std::string& _source);
		void print_tokens();

	};

}
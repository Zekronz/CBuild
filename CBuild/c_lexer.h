#pragma once

#include <string>
#include <vector>

#include "types.h"
#include "error_handler.h"

namespace CBuild {

	struct Error_Handler;

	enum class C_Token_Type : u8 {

		None,
		Identifier,
		Directive,
		ValueType,
		Keyword,
		String,
		Include_String,
		Char_Litteral,
		Integer,
		Float,
		Dot,
		Comma,
		Colon,
		Semicolon,
		OpenPar,
		ClosePar,
		OpenSquare,
		CloseSquare,
		OpenCurly,
		CloseCurly,
		Operator,
		End_Of_File,

	};

	struct C_Token {

		C_Token_Type type = C_Token_Type::None;
		std::string value = "";

		s64 line_pos = -1;
		s64 char_pos = -1;

	};

	enum class C_Lexer_State : u8 {

		Normal,
		Single_Line_Comment,
		Multi_Line_Comment,
		String,
		Include_String,
		Number,
		Keyword,

	};

	struct C_Lexer {

		std::vector<C_Token> tokens;
		Error_Handler error_handler;

		bool is_digit(s8 _char);
		bool is_letter(s8 _char);
		bool is_keyword_char(s8 _char);
		bool is_keyword(const std::string& _keyword);
		s8 char_at(const std::string& _source, size_t _index);
		bool parse_source(const std::string& _source);
		void clear();
		void print_tokens();

	};

}
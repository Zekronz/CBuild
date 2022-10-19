#include "pch.h"
#include "lexer.h"

namespace CBuild {

	bool Lexer::is_valid_path_char(s8 _char) {

		if (_char == '/' || _char == '\\' || _char == '?' || _char == '%' || _char == '*' || _char == ':' || _char == '|' || _char == '"' || _char == '<' || _char == '>' || _char == ',' || _char == ';' || _char == '=' || _char == ' ' || _char == '\t' || _char == '\n' || _char == '\r') {
			return false;
		}

		return true;

	}

	bool Lexer::is_digit(s8 _char) {
		return (_char >= '0' && _char <= '9');
	}

	bool Lexer::is_letter(s8 _char) {
		return (_char >= 'A' && _char <= 'Z') || (_char >= 'a' && _char <= 'z');
	}

	bool Lexer::is_string_char(s8 _char) {

		if (/*_char == '/' || _char == '\\' || */_char == '?' || _char == '%' || _char == '*' || _char == ':' || _char == '|' || _char == '"' || _char == '<' || _char == '>' || _char == ',' || _char == ';' || _char == '=' || _char == ' ' || _char == '\t' || _char == '\n' || _char == '\r') {
			return false;
		}

		return true;

		//return (_char == '_') || is_digit(_char) || is_letter(_char);

	}

	s8 Lexer::char_at(const std::string& _source, size_t _index) {

		if (_source.length() <= 0 || _index >= _source.length()) return 0;
		return _source[_index];

	}

	bool Lexer::parse_source(const std::string& _source) {

		std::string source = _source + " ";
		tokens.clear();

		Lexer_State state = Lexer_State::Normal;
		std::string token_name = "";

		size_t nested_comment_count = 0;

		s64 line_pos = 1;
		s64 char_pos = 1;
		s64 prev_char_pos = 1;
		s64 token_line_pos = -1;
		s64 token_char_pos = -1;
		bool new_line = false;
		bool increment_char_pos = true;

		size_t length = source.length();
		for (size_t i = 0; i < length; ++i) {

			s8 cur = source[i];

			if (cur == '\n') {

				++line_pos;
				prev_char_pos = char_pos;
				char_pos = 1;
				new_line = true;

			}

			//Normal state.
			if (state == Lexer_State::Normal) {

				//Slash.
				if (cur == '/') {

					s8 next = char_at(source, i + 1);

					//Single line comment.
					if (next == '/') {

						state = Lexer_State::Single_Line_Comment;

						++i;
						++char_pos;

					}

					//Multi line comment.
					else if (next == '*') {

						state = Lexer_State::Multi_Line_Comment;
						nested_comment_count = 1;

						++i;
						++char_pos;

					}

					//String.
					else {

						state = Lexer_State::String;
						token_name = cur;

						token_line_pos = line_pos;
						token_char_pos = char_pos;

					}

				}

				//String.
				else if (is_string_char(cur)) {

					state = Lexer_State::String;
					token_name = cur;

					token_line_pos = line_pos;
					token_char_pos = char_pos;

				}

				//Semicolon.
				else if (cur == ';') {
					tokens.push_back({ Token_Type::Semicolon, ";", line_pos, char_pos });
				}

				//Invalid character.
				else if (cur != ' ' && cur != '\t' && cur != '\n' && cur != '\r') {

					std::string msg = "Unexpected symbol found: '";
					msg += cur;
					msg += "'";

					error_handler.set_error(Error_Type::Syntax_Error, msg, line_pos, char_pos);
					return false;

				}

			}

			//String state.
			else if (state == Lexer_State::String) {

				if (!is_string_char(cur)) {

					state = Lexer_State::Normal;

					--i;
					if (new_line) {

						--line_pos;
						char_pos = prev_char_pos;
						new_line = false;
						increment_char_pos = false;

					}
					else {
						--char_pos;
					}

					tokens.push_back({ Token_Type::String, token_name, token_line_pos, token_char_pos});

				}
				else {
					token_name += cur;
				}

			}

			//Single line comment state.
			else if (state == Lexer_State::Single_Line_Comment) {

				if (cur == '\n') {
					state = Lexer_State::Normal;
				}

			}

			//Multi line comment state.
			else if (state == Lexer_State::Multi_Line_Comment) {

				if (cur == '/') {

					s8 next = char_at(source, i + 1);

					if (next == '*') {

						++nested_comment_count;
						++i;
						++char_pos;

					}

				}
				else if (cur == '*') {

					s8 next = char_at(source, i + 1);

					if (next == '/') {

						--nested_comment_count;
						++i;
						++char_pos;

						if (nested_comment_count == 0) {

							state = Lexer_State::Normal;

						}

					}

				}

			}

			if (!new_line && increment_char_pos) {
				++char_pos;
			}

			new_line = false;
			increment_char_pos= true;

		}

		tokens.push_back({ Token_Type::End_Of_File, "EOF", line_pos, char_pos - 1 });

		return true;

	}

	void Lexer::print_tokens() {

		for (const Token& token : tokens) {

			if (token.type == Token_Type::None) std::cout << "None: ";
			else if (token.type == Token_Type::String) std::cout << "String: ";
			else if (token.type == Token_Type::Semicolon) std::cout << "Semicolon: ";

			std::cout << token.value << std::endl;

		}

	}

}
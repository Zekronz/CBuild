#include "pch.h"
#include "c_lexer.h"

namespace CBuild {

	bool C_Lexer::is_digit(s8 _char) {
		return (_char >= '0' && _char <= '9');
	}

	bool C_Lexer::is_letter(s8 _char) {
		return (_char >= 'A' && _char <= 'Z') || (_char >= 'a' && _char <= 'z');
	}

	bool C_Lexer::is_keyword_char(s8 _char) {
		return (_char == '_') || is_digit(_char) || is_letter(_char);
	}

	bool C_Lexer::is_keyword(const std::string& _str) {
		return false;
	}

	s8 C_Lexer::char_at(const std::string& _source, size_t _index) {

		if (_source.length() <= 0 || _index >= _source.length()) return 0;
		return _source[_index];

	}

	bool C_Lexer::parse_source(const std::string& _source) {

		std::string source = _source + " ";
		tokens.clear();

		C_Lexer_State state = C_Lexer_State::Normal;
		std::string token_name = "";

		bool has_decimal = false;
		bool is_char_litteral = false;

		bool check_for_include_string = false;
		s64 include_string_start = 0;
		s64 include_string_char_pos = 0;

		size_t par_count = 0;
		size_t curly_count = 0;
		size_t square_count = 0;

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

			//New line.
			if (cur == '\n') {

				if (check_for_include_string) {

					check_for_include_string = false;
					i = include_string_start;
					char_pos = include_string_char_pos;

					token_name = "";
					state = C_Lexer_State::Normal;

					cur = source[i];

				}
				else {

					++line_pos;
					prev_char_pos = char_pos;
					char_pos = 1;
					new_line = true;

				}

			}

			//Normal state.
			if (state == C_Lexer_State::Normal) {

				//Keyword.
				if ((is_keyword_char(cur) && !is_digit(cur)) || cur == '#') {

					state = C_Lexer_State::Keyword;
					token_name = cur;

					token_line_pos = line_pos;
					token_char_pos = char_pos;

				}

				//String.
				else if (cur == '"' || cur == '\'') {

					state = C_Lexer_State::String;
					is_char_litteral = (cur == '\'');
					token_name = "";

					token_line_pos = line_pos;
					token_char_pos = char_pos;

				}

				//Number.
				else if (is_digit(cur)) {

					state = C_Lexer_State::Number;

					has_decimal = false;
					token_name = cur;

					token_line_pos = line_pos;
					token_char_pos = char_pos;

				}

				//Dot.
				else if (cur == '.') {

					s8 next = char_at(source, i + 1);

					//Number.
					if (is_digit(next)) {

						state = C_Lexer_State::Number;
						++i;
						++char_pos;

						has_decimal = true;
						token_name = '.';
						token_name += next;

						token_line_pos = line_pos;
						token_char_pos = char_pos;

					}

					//Dot.
					else {
						tokens.push_back({ C_Token_Type::Dot, ".", line_pos, char_pos });
					}

				}

				//Comma.
				else if (cur == ',') {
					tokens.push_back({ C_Token_Type::Comma, ",", line_pos, char_pos });
				}

				//Colon.
				else if (cur == ':') {
					tokens.push_back({ C_Token_Type::Colon, ":", line_pos, char_pos });
				}

				//Semicolon.
				else if (cur == ';') {
					tokens.push_back({ C_Token_Type::Semicolon, ";", line_pos, char_pos });
				}

				//Opening parenthesis.
				else if (cur == '(') {

					tokens.push_back({ C_Token_Type::OpenPar, "(", line_pos, char_pos });
					++par_count;

				}

				//Closing parenthesis.
				else if (cur == ')') {
					tokens.push_back({ C_Token_Type::ClosePar, ")", line_pos, char_pos });
				}

				//Opening square bracket.
				else if (cur == '[') {

					tokens.push_back({ C_Token_Type::OpenSquare, "[", line_pos, char_pos });
					++square_count;

				}
				
				//Closing square bracket.
				else if (cur == ']') {
					tokens.push_back({ C_Token_Type::CloseSquare, "]", line_pos, char_pos });
				}

				//Opening curly bracket.
				else if (cur == '{') {

					tokens.push_back({ C_Token_Type::OpenCurly, "{", line_pos, char_pos });
					++curly_count;

				}
				
				//Closing curly bracket.
				else if (cur == '}') {
					tokens.push_back({ C_Token_Type::CloseCurly, "}", line_pos, char_pos });
				}

				//Plus.
				else if (cur == '+') {

					s8 next = char_at(source, i + 1);

					//Plus plus.
					if (next == '+') {

						tokens.push_back({ C_Token_Type::Operator, "++", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Plus equals.
					else if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "+=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Plus.
					else {
						tokens.push_back({ C_Token_Type::Operator, "+", line_pos, char_pos });
					}
				}

				//Minus.
				else if (cur == '-') {

					s8 next = char_at(source, i + 1);

					//Mins minus.
					if (next == '-') {

						tokens.push_back({ C_Token_Type::Operator, "--", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Mins equals.
					else if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "-=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Minus.
					else {
						tokens.push_back({ C_Token_Type::Operator, "-", line_pos, char_pos });
					}
				}

				//Multiply.
				else if (cur == '*') {

					s8 next = char_at(source, i + 1);

					//Multiply equals.
					if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "*=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Multiply.
					else {
						tokens.push_back({ C_Token_Type::Operator, "*", line_pos, char_pos });
					}
				}

				//Modulus.
				else if (cur == '%') {
					
					s8 next = char_at(source, i + 1);

					//Modulus equals.
					if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "%=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Modulus.
					else {
						tokens.push_back({ C_Token_Type::Operator, "%", line_pos, char_pos });
					}

				}

				//Equals.
				else if (cur == '=') {

					s8 next = char_at(source, i + 1);

					//Equals equals.
					if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "==", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Equals.
					else {
						tokens.push_back({ C_Token_Type::Operator, "=", line_pos, char_pos });
					}

				}

				//Not.
				else if (cur == '!') {

					s8 next = char_at(source, i + 1);

					//Not equals.
					if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "!=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Not.
					else {
						tokens.push_back({ C_Token_Type::Operator, "!", line_pos, char_pos });
					}

				}

				//Greater.
				else if (cur == '>') {

					s8 next = char_at(source, i + 1);

					//Greater equals.
					if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, ">=", line_pos, char_pos });

						++i;
						++char_pos;

					}
					else if (next == '>') {

						next = char_at(source, i + 1);

						//Bitwise right shift equals.
						if (next == '=') {

							tokens.push_back({ C_Token_Type::Operator, ">>=", line_pos, char_pos });

							++i;
							++char_pos;

						}

						//Bitwise right shift.
						else {
							tokens.push_back({ C_Token_Type::Operator, ">>", line_pos, char_pos });
						}

						++i;
						++char_pos;

					}

					//Greater.
					else {
						tokens.push_back({ C_Token_Type::Operator, ">", line_pos, char_pos });
					}

				}

				//Less.
				else if (cur == '<') {

					s8 next = char_at(source, i + 1);

					//Less equals.
					if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "<=", line_pos, char_pos });

						++i;
						++char_pos;

					}
					else if (next == '<') {

						next = char_at(source, i + 1);

						//Bitwise left shift equals.
						if (next == '=') {

							tokens.push_back({ C_Token_Type::Operator, "<<=", line_pos, char_pos });

							++i;
							++char_pos;

						}

						//Bitwise left shift.
						else {
							tokens.push_back({ C_Token_Type::Operator, "<<", line_pos, char_pos });
						}

						++i;
						++char_pos;

					}
					else {

						//Include string.
						if (check_for_include_string) {

							state = C_Lexer_State::Include_String;
							token_name = "";

							token_line_pos = line_pos;
							token_char_pos = char_pos;

						}

						//Less.
						else {
							tokens.push_back({ C_Token_Type::Operator, "<", line_pos, char_pos });
						}

					}
					
				}

				//And.
				else if (cur == '&') {

					s8 next = char_at(source, i + 1);

					//And and.
					if (next == '&') {

						tokens.push_back({ C_Token_Type::Operator, "&&", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//And equals.
					else if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "&=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//And.
					else {
						tokens.push_back({ C_Token_Type::Operator, "&", line_pos, char_pos });
					}

				}

				//Or.
				else if (cur == '|') {

					s8 next = char_at(source, i + 1);

					//Or or.
					if (next == '|') {

						tokens.push_back({ C_Token_Type::Operator, "||", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Or equals.
					else if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "|=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Or.
					else {
						tokens.push_back({ C_Token_Type::Operator, "|", line_pos, char_pos });
					}

				}

				//Bitwise ^ .
				else if (cur == '^') {

					s8 next = char_at(source, i + 1);

					//Bitwise ^= .
					if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "^=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Bitwise ^ .
					else {
						tokens.push_back({ C_Token_Type::Operator, "^", line_pos, char_pos });
					}

				}

				//Bitwise ~
				else if (cur == '~') {
					tokens.push_back({ C_Token_Type::Operator, "~", line_pos, char_pos });
				}

				//Conditional ?
				else if (cur == '?') {
					tokens.push_back({ C_Token_Type::Operator, "?", line_pos, char_pos });
				}

				//Slash.
				else if (cur == '/') {

					s8 next = char_at(source, i + 1);

					//Single line comment.
					if (next == '/') {

						state = C_Lexer_State::Single_Line_Comment;

						++i;
						++char_pos;

					}

					//Multi line comment.
					else if (next == '*') {

						state = C_Lexer_State::Multi_Line_Comment;

						++i;
						++char_pos;

					}

					//Divide-equals.
					else if (next == '=') {

						tokens.push_back({ C_Token_Type::Operator, "/=", line_pos, char_pos });

						++i;
						++char_pos;

					}

					//Division.
					else {
						tokens.push_back({ C_Token_Type::Operator, "/", line_pos, char_pos });
					}

				}

				//Ignore any other characters.
				else {

				}

			}

			//Keyword state.
			else if (state == C_Lexer_State::Keyword) {

				if (!is_keyword_char(cur)) {

					state = C_Lexer_State::Normal;

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

					C_Token_Type type = token_name[0] == '#' ? C_Token_Type::Directive : C_Token_Type::Identifier;
					tokens.push_back({ type, token_name, token_line_pos, token_char_pos });

					if (token_name == "#include" && !new_line) {

						check_for_include_string = true;
						include_string_start = i + 1;
						include_string_char_pos = char_pos + 1;

					}

				}
				else {
					token_name += cur;
				}

			}

			//String state.
			else if (state == C_Lexer_State::String) {

				if ((cur == '"' && !is_char_litteral) || (cur == '\'' && is_char_litteral)) {

					state = C_Lexer_State::Normal;
					tokens.push_back({ is_char_litteral ? C_Token_Type::Char_Litteral : C_Token_Type::String, token_name, token_line_pos, token_char_pos });

				}
				else {
					token_name += cur;
				}

			}

			//Include_String state.
			else if (state == C_Lexer_State::Include_String) {

				if (cur == '>') {

					check_for_include_string = false;
					state = C_Lexer_State::Normal;
					tokens.push_back({ C_Token_Type::Include_String, token_name, token_line_pos, token_char_pos });

				}
				else {
					token_name += cur;
				}

			}

			//Number.
			else if (state == C_Lexer_State::Number) {

				if (is_digit(cur) || (cur == '.' && !has_decimal)) {

					token_name += cur;
					if (cur == '.') has_decimal = true;

				}
				else {

					if (token_name[0] == '.') {
						token_name.insert(token_name.begin(), '0');
					}

					if (token_name[token_name.size() - 1] == '.') {
						token_name += '0';
					}

					state = C_Lexer_State::Normal;
					tokens.push_back({ has_decimal ? C_Token_Type::Float : C_Token_Type::Integer, token_name, token_line_pos, token_char_pos });

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

				}

			}

			//Single line comment state.
			else if (state == C_Lexer_State::Single_Line_Comment) {

				if (cur == '\n') {
					state = C_Lexer_State::Normal;
				}

			}

			//Multi line comment state.
			else if (state == C_Lexer_State::Multi_Line_Comment) {

				if (cur == '*') {

					s8 next = char_at(source, i + 1);

					if (next == '/') {

						++i;
						++char_pos;

						state = C_Lexer_State::Normal;

					}

				}

			}

			if (!new_line && increment_char_pos) {
				++char_pos;
			}

			new_line = false;
			increment_char_pos = true;

		}

		tokens.push_back({ C_Token_Type::End_Of_File, "EOF", line_pos, char_pos - 1 });

		return true;

	}

	void C_Lexer::clear() {

		tokens.clear();
		error_handler.clear_error();

	}

	void C_Lexer::print_tokens() {

		for (const C_Token& token : tokens) {

			if (token.type == C_Token_Type::None) std::cout << "None: ";
			else if (token.type == C_Token_Type::Identifier) std::cout << "ID: ";
			else if (token.type == C_Token_Type::Directive) std::cout << "Directive: ";
			else if (token.type == C_Token_Type::Keyword) std::cout << "Keyword: ";
			else if (token.type == C_Token_Type::String) std::cout << "String: ";
			else if (token.type == C_Token_Type::Include_String) std::cout << "Include_String: ";
			else if (token.type == C_Token_Type::Char_Litteral) std::cout << "Char: ";
			else if (token.type == C_Token_Type::Integer) std::cout << "Integer: ";
			else if (token.type == C_Token_Type::Float) std::cout << "Float: ";
			else if (token.type == C_Token_Type::Operator) std::cout << "Operator: ";
			else if (token.type == C_Token_Type::Dot) std::cout << "Dot: ";
			else if (token.type == C_Token_Type::Comma) std::cout << "Comma: ";
			else if (token.type == C_Token_Type::Colon) std::cout << "Colon: ";
			else if (token.type == C_Token_Type::Semicolon) std::cout << "Semicolon: ";
			else if (token.type == C_Token_Type::OpenPar) std::cout << "Open_Par: ";
			else if (token.type == C_Token_Type::ClosePar) std::cout << "Close_Par: ";
			else if (token.type == C_Token_Type::OpenCurly) std::cout << "Open_Curly: ";
			else if (token.type == C_Token_Type::CloseCurly) std::cout << "Close_Curly: ";
			else if (token.type == C_Token_Type::OpenSquare) std::cout << "Open_Square: ";
			else if (token.type == C_Token_Type::CloseSquare) std::cout << "Close_Square: ";
			else if (token.type == C_Token_Type::End_Of_File) std::cout << "End_Of_File: ";

			std::cout << token.value << std::endl;

		}

	}

}
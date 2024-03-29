#include "pch.h"
#include "parser.h"
#include "lexer.h"
#include "log.h"
#include "file.h"

#include <filesystem>

#define COMMAND_FUNC(func) std::bind(&func, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

namespace CBuild {

	Parser::Parser() {

		//Commands.
		cmds["set_compiler"]			= { COMMAND_FUNC(Parser::parse_cmd_set_compiler) };
		cmds["set_compiler_dir"]		= { COMMAND_FUNC(Parser::parse_cmd_set_compiler_dir) };
		cmds["set_project_name"]		= { COMMAND_FUNC(Parser::parse_cmd_set_project_name) };
		cmds["set_avr_mcu"]				= { COMMAND_FUNC(Parser::parse_cmd_set_avr_mcu) };
		cmds["set_atmel_studio_dir"]	= { COMMAND_FUNC(Parser::parse_cmd_set_atmel_studio_dir) };
		cmds["set_build_type"]			= { COMMAND_FUNC(Parser::parse_cmd_set_build_type) };
		cmds["set_build_output"]		= { COMMAND_FUNC(Parser::parse_cmd_set_build_output) };
		cmds["set_build_name"]			= { COMMAND_FUNC(Parser::parse_cmd_set_build_name) };
		cmds["set_obj_output"]			= { COMMAND_FUNC(Parser::parse_cmd_set_obj_output) };
		cmds["set_precompiled_header"]	= { COMMAND_FUNC(Parser::parse_cmd_set_precompiled_header) };
		cmds["set_pch"]					= { COMMAND_FUNC(Parser::parse_cmd_set_precompiled_header) };
		cmds["set_run_binary"]			= { COMMAND_FUNC(Parser::parse_cmd_set_run_binary) };
		cmds["add_src_dirs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_src_dirs) };
		cmds["add_src_files"]			= { COMMAND_FUNC(Parser::parse_cmd_add_src_files) };
		cmds["add_incl_dirs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_incl_dirs) };
		cmds["add_lib_dirs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_lib_dirs) };
		cmds["add_static_libs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_static_libs) };

		//Compiler specs.
		compiler_specs["gcc"] = new Compiler_Spec_GCC();
		compiler_specs["avr-gcc"] = new Compiler_Spec_AVR_GCC();
		compiler_specs["clang"] = new Compiler_Spec_Clang();

	}

	Parser::~Parser() {

		for (const auto& it : compiler_specs) {
			delete it.second;
		}

	}

	bool Parser::get_current_token(const u64 _index, Token& _cur_token) {

		if (lexer == nullptr) return false;
		if (_index >= lexer->tokens.size()) return false;

		_cur_token = lexer->tokens[_index];

		return true;

	}

	bool Parser::get_next_token(u64& _index, Token& _cur_token, Token& _prev_token) {

		if (lexer == nullptr) return false;
		if (_index >= lexer->tokens.size()) return false;

		_prev_token = _cur_token;
		_cur_token = lexer->tokens[_index];
		++_index;

		return true;

	}

	bool Parser::get_prev_token(u64& _index, Token& _cur_token, Token& _prev_token) {

		if (lexer == nullptr) return false;
		if (_index <= 0) return false;

		_cur_token = _prev_token;
		--_index;

		if (_index > 0) {
			_prev_token = lexer->tokens[_index - 1];
		}
		else {
			_prev_token = {};
		}

		return true;

	}

	bool Parser::parse_tokens(Lexer* _lexer) {

		lexer = _lexer;

		u64 length = lexer->tokens.size();
		if (length <= 0) return true;

		u64 i = 0;
		Token token, prev_token;

		while (get_next_token(i, token, prev_token)) {

			//End of file.
			if (token.type == Token_Type::End_Of_File) {

				if (project_name.empty()) {
					project_name = std::filesystem::current_path().stem().string();
				}

				if (build_name.empty()) {
					build_name = project_name;
				}

				return true;

			}

			//Command.
			else if (token.type == Token_Type::Command) {
				if (!parse_cmd(i, token, prev_token)) return false;
			}

			//Unexpected token.
			else {

				std::string msg = "Unexpected token found: '" + token.value + "'";
				error_handler.set_error(Error_Type::Error, msg, token.line_pos, token.char_pos);
				return false;

			}

		}

		error_handler.set_error(Error_Type::Error, "Error in Parser::parse_tokens.", -1, -1);
		return false;

	}

	bool Parser::parse_semicolon(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type == Token_Type::Semicolon) return true;

		std::string msg = "Symbol ';' expected, got '" + _cur_token.value + "'";
		error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
		return false;

	}

	bool Parser::parse_cmd(u64& _index, Token& _cur_token, Token& _prev_token) {

		const auto& it = cmds.find(_cur_token.value);
		if (it == cmds.end()) {

			std::string msg = "Invalid command found: '" + _cur_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (!it->second.callback(_index, _cur_token, _prev_token)) return false;

		return true;

	}

	bool Parser::parse_cmd_set_compiler(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'compiler_name' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}
		
		compiler = _cur_token.value;
		String_Helper::lower(compiler);

		const auto& spec_it = compiler_specs.find(compiler);

		if (spec_it == compiler_specs.end()) {

			std::string msg = "Invalid compiler '" + _cur_token.value + "' specified in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_compiler_dir(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'compiler_dir' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (!lexer->is_valid_path_string(_cur_token.value)) {

			std::string msg = "Invalid directory '" + _cur_token.value + "' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		compiler_dir = std::filesystem::u8path(_cur_token.value);
		File::format_path(compiler_dir);
		
		if (!File::directory_exists(compiler_dir)) {

			std::string msg = "Compiler directory does not exist: '" + _cur_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_avr_mcu(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'mcu' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		avr_mcu = _cur_token.value;
		String_Helper::lower(avr_mcu);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_atmel_studio_dir(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'ateml_studio_dir' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (!lexer->is_valid_path_string(_cur_token.value)) {

			std::string msg = "Invalid directory '" + _cur_token.value + "' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		atmel_studio_dir = std::filesystem::u8path(_cur_token.value);
		File::format_path(atmel_studio_dir);

		if (!File::directory_exists(atmel_studio_dir)) {

			std::string msg = "Directory does not exist: '" + _cur_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_build_type(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'build_type' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		std::string build_type_name = _cur_token.value;
		String_Helper::lower(build_type_name);

		if (build_type_name != "binary" && build_type_name != "static_lib") {

			std::string msg = "Invalid build_type '" + _cur_token.value + "' specified in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (build_type_name == "binary") build_type = Build_Type::Binary;
		else build_type = Build_Type::Static_Lib;

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_project_name(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'project_name' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (!lexer->is_valid_file_name_string(_cur_token.value)) {

			std::string msg = "Invalid project name '" + _cur_token.value + "' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		project_name = _cur_token.value;

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_obj_output(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'obj_output_dir' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (!lexer->is_valid_path_string(_cur_token.value)) {

			std::string msg = "Invalid directory '" + _cur_token.value + "' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		obj_output = std::filesystem::u8path(_cur_token.value);
		File::format_path(obj_output);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_build_output(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'build_output_dir' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (!lexer->is_valid_path_string(_cur_token.value)) {

			std::string msg = "Invalid directory '" + _cur_token.value + "' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		build_output = std::filesystem::u8path(_cur_token.value);
		File::format_path(build_output);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_build_name(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'build_name' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}
		
		if (!lexer->is_valid_file_name_string(_cur_token.value)) {

			std::string msg = "Invalid build name '" + _cur_token.value + "' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		build_name = _cur_token.value;

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_precompiled_header(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'precompiled_header' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (!lexer->is_valid_path_string(_cur_token.value)) {

			std::string msg = "Invalid file path '" + _cur_token.value + "' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		precompiled_header = std::filesystem::u8path(_cur_token.value);
		File::format_path(precompiled_header);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_run_binary(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::Bool) {

			std::string msg = "Expected boolean argument 'run_binary' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		std::string run = _cur_token.value;

		run_binary = (run == "true") ? true : false;

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_add_src_dirs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_dirs(_index, _cur_token, _prev_token, src_dirs);
	}

	bool Parser::parse_cmd_add_src_files(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_files(_index, _cur_token, _prev_token, src_files);
	}

	bool Parser::parse_cmd_add_incl_dirs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_dirs(_index, _cur_token, _prev_token, incl_dirs);
	}

	bool Parser::parse_cmd_add_lib_dirs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_dirs(_index, _cur_token, _prev_token, lib_dirs);
	}

	bool Parser::parse_cmd_add_static_libs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_strings(_index, _cur_token, _prev_token, static_libs, true);
	}

	bool Parser::parse_cmd_add_dirs(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::filesystem::path>& _dirs) {

		Token cmd_token = _cur_token;

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected at least 1 'dir' argument in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		while (_cur_token.type == Token_Type::String) {

			if (!lexer->is_valid_path_string(_cur_token.value)) {

				std::string msg = "Invalid directory '" + _cur_token.value + "' in command '" + cmd_token.value + "'";
				error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
				return false;

			}

			bool exists = false;

			std::filesystem::path dir_path = std::filesystem::u8path(_cur_token.value);
			File::format_path(dir_path);

			for (const std::filesystem::path& path : _dirs) {
				
				if (File::compare(path, dir_path)) {

					exists = true;
					break;

				}

			}

			if (exists) {

				std::string msg = "Directory '" + _cur_token.value + "' has already been added in command '" + cmd_token.value + "'";
				error_handler.warn(msg, _cur_token.line_pos, _cur_token.char_pos);

			}
			else {
				_dirs.push_back(dir_path);
			}

			get_next_token(_index, _cur_token, _prev_token);

		}

		get_prev_token(_index, _cur_token, _prev_token);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_add_files(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::filesystem::path>& _files) {

		Token cmd_token = _cur_token;

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected at least 1 'file' argument in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		while (_cur_token.type == Token_Type::String) {

			if (!lexer->is_valid_path_string(_cur_token.value)) {

				std::string msg = "Invalid file path '" + _cur_token.value + "' in command '" + cmd_token.value + "'";
				error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
				return false;

			}

			bool exists = false;

			std::filesystem::path file_path = std::filesystem::u8path(_cur_token.value);
			File::format_path(file_path);

			if (!File::file_exists(file_path)) {

				std::string msg = "File '" + _cur_token.value + "' does not exist, in command '" + cmd_token.value + "'";
				error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
				return false;

			}

			for (const std::filesystem::path& path : _files) {

				if (File::compare(path, file_path)) {

					exists = true;
					break;

				}

			}

			if (exists) {

				std::string msg = "File '" + _cur_token.value + "' has already been added in command '" + cmd_token.value + "'";
				error_handler.warn(msg, _cur_token.line_pos, _cur_token.char_pos);

			}
			else {
				_files.push_back(file_path);
			}

			get_next_token(_index, _cur_token, _prev_token);

		}

		get_prev_token(_index, _cur_token, _prev_token);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_add_strings(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::string>& _strings, bool _validate_strings) {

		Token cmd_token = _cur_token;

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected at least 1 'string' argument in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		while (_cur_token.type == Token_Type::String) {

			if (_validate_strings && !lexer->is_valid_file_name_string(_cur_token.value)) {

				std::string msg = "Invalid string '" + _cur_token.value + "' in command '" + cmd_token.value + "'";
				error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
				return false;

			}

			bool exists = false;

			for (const std::string& str : _strings) {

				if (_cur_token.value == str) {

					exists = true;
					break;

				}

			}

			if (exists) {

				std::string msg = "String '" + _cur_token.value + "' has already been added in command '" + cmd_token.value + "'";
				error_handler.warn(msg, _cur_token.line_pos, _cur_token.char_pos);

			}
			else {
				_strings.push_back(_cur_token.value);
			}

			get_next_token(_index, _cur_token, _prev_token);

		}

		get_prev_token(_index, _cur_token, _prev_token);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_source_and_header_files(const std::filesystem::path& _path, Config_Type _config_type, const std::string& _compiler) {

		//@TODO: Standard libraries?

		//Check if the file has already been parsed.
		for (const Checked_File& checked_file : checked_files) {

			if (File::compare(checked_file.path, _path)) {
				return checked_file.rebuild;
			}

		}

		bool should_rebuild = (config.last_used_compiler != _compiler);

		if (!File::file_exists(_path)) {

			checked_files.push_back({ _path, false, 0 });
			return false;

		}

		//Rebuild source file if object file does not exist.
		std::filesystem::path obj_output_path = get_obj_output_path(_config_type);

		if (!should_rebuild && _path.extension().string() == ".c") {

			std::filesystem::path obj_file_path = obj_output_path / _path.filename().replace_extension(".o");
			File::format_path(obj_file_path);

			if (!File::file_exists(obj_file_path)) should_rebuild = true;

		}

		//Compare timestamps.
		//@TODO: Check checksum instead?
		Config_Timestamps* timestamps = config.get_config_timestamps(_config_type);

		u64 time = std::filesystem::last_write_time(_path).time_since_epoch().count();

		if (!should_rebuild) {

			if (timestamps != nullptr && timestamps->timestamps.find(_path.string()) != timestamps->timestamps.end()) {

				u64 old_time = timestamps->timestamps[_path.string()];
				if (old_time != time) should_rebuild = true;

			}
			else {
				should_rebuild = true;
			}

		}

		//Parse include directives in file.
		std::string source;
		if (!File::read_text_file(_path, source)) {
			checked_files.push_back({ _path, false, time });
			return false;
		}

		std::vector<std::string> local_files;
		std::vector<std::string> include_files;
		c_lexer.clear();
		c_lexer.parse_source(source);

		for (u64 i = 0; i < c_lexer.include_indices.size(); ++i) {

			u64 ind = c_lexer.include_indices[i];

			C_Token& token = c_lexer.tokens[ind];

			++ind;
			if (ind >= c_lexer.tokens.size()) continue;

			token = c_lexer.tokens[ind];

			if (token.type == C_Token_Type::String) {
				local_files.push_back(token.value);
			}
			else if (token.type == C_Token_Type::Include_String) {
				include_files.push_back(token.value);
			}

		}

		for (const std::string& local_file : local_files) {

			std::filesystem::path local_path = _path.has_parent_path() ? _path.parent_path() / std::filesystem::u8path(local_file) : std::filesystem::u8path(local_file);
			File::format_path(local_path);
			
			//Fallback to include.
			if (!File::file_exists(local_path)) {

				bool add = true;
				for (const std::string& temp : include_files) {
					if (File::compare(temp, local_file)) {
						add = false;
						break;
					}
				}

				if (add) include_files.push_back(local_file);

				continue;

			}

			if (File::compare(_path, local_path)) continue;
			if (parse_source_and_header_files(local_path, _config_type, _compiler)) should_rebuild = true;

		}

		std::string incl_path = "";
		for (const std::string& include_file : include_files) {

			for (const std::filesystem::path& include_dir : incl_dirs) {

				std::filesystem::path incl_path = include_dir / std::filesystem::u8path(include_file);
				File::format_path(incl_path);

				if (!File::file_exists(incl_path)) continue;
				if (File::compare(_path, incl_path)) continue;
				if (parse_source_and_header_files(incl_path, _config_type, _compiler)) should_rebuild = true;

			}

		}

		checked_files.push_back({ _path, should_rebuild, time });
		return should_rebuild;

	}

	std::filesystem::path Parser::get_atmel_studio_include_path() {

		std::filesystem::path path = atmel_studio_dir / std::filesystem::u8path("Packs\\atmel\\ATmega_DFP\\1.6.364\\include");
		File::format_path(path);

		return path;

	}

	std::filesystem::path Parser::get_atmel_studio_mcu_path() {

		std::filesystem::path path = atmel_studio_dir / std::filesystem::u8path("Packs\\atmel\\ATmega_DFP\\1.6.364\\gcc\\dev\\" + avr_mcu);
		File::format_path(path);

		return path;

	}

	std::filesystem::path Parser::get_obj_output_path(Config_Type _config_type) {
		return obj_output / std::filesystem::u8path(config.config_type_to_string(_config_type));
	}

	std::filesystem::path Parser::get_build_output_path(Config_Type _config_type) {
		return build_output / std::filesystem::u8path(config.config_type_to_string(_config_type));
	}

	std::filesystem::path Parser::get_compiler_path(const std::string _name) {

		std::filesystem::path path = compiler_dir;
		path /= std::filesystem::u8path(_name);
		File::format_path(path);

		return path;

	}

	bool Parser::should_build() {
		return (src_dirs.size() > 0 || src_files.size() > 0);
	}

	bool Parser::build(const std::filesystem::path& _projects_path, bool _force_rebuild, bool _print_cmds, Config_Type _config_type) {

		exec_path = _projects_path.parent_path();
		
		if (compiler == "gcc" || compiler == "avr-gcc" || compiler == "clang") {
			return build_gcc_clang(compiler, _projects_path, _force_rebuild, _print_cmds, _config_type);
		}

		return true;

	}

	bool Parser::build_gcc_clang(const std::string& _compiler, const std::filesystem::path& _projects_path, bool _force_rebuild, bool _print_cmds, Config_Type _config_type) {

		//@TODO: Display what compiler is used and time measurment.
		//@TODO: Reset to white.
		//@TODO: Check if library updated.
		//@TODO: Check timestamp of build file?
		
		std::string cmd;

		const auto& spec_it = compiler_specs.find(_compiler);
		if (spec_it == compiler_specs.end()) {

			CBUILD_ERROR("Invalid compiler: {}", _compiler);
			return false;

		}

		Compiler_Spec* compiler = spec_it->second;

		//Check AVR-GCC directories.
		if (compiler->type == Compiler_Type::AVR_GCC) {

			std::filesystem::path atmel_studio_include_dir = get_atmel_studio_include_path();
			std::filesystem::path atmel_studio_mcu_dir = get_atmel_studio_mcu_path();

			if (atmel_studio_dir.empty()) {

				CBUILD_ERROR("No Atmel Studio directory specified.");
				return false;

			}

			if (!File::directory_exists(atmel_studio_dir)) {

				CBUILD_ERROR("Atmel Studio directory does not exist: {}", atmel_studio_dir.string());
				return false;

			}

			if (!File::directory_exists(atmel_studio_include_dir)) {

				CBUILD_ERROR("Atmel Studio include directory does not exist: {}", atmel_studio_include_dir.string());
				return false;

			}

			if (!File::directory_exists(atmel_studio_mcu_dir)) {

				CBUILD_ERROR("Atmel Studio '{}' directory does not exist: {}", avr_mcu, atmel_studio_mcu_dir.string());
				return false;

			}

		}

		//Create obj and build directories.
		std::filesystem::path obj_output_path = get_obj_output_path(_config_type);
		std::filesystem::path build_output_path = get_build_output_path(_config_type);

		if (!File::directory_exists(obj_output)) {
			std::filesystem::create_directory(obj_output);
		}

		if (!File::directory_exists(obj_output_path)) {
			std::filesystem::create_directory(obj_output_path);
		}

		if (!File::directory_exists(build_output)) {
			std::filesystem::create_directory(build_output);
		}

		if (!File::directory_exists(build_output_path)) {
			std::filesystem::create_directory(build_output_path);
		}

		bool built_something = false;

		//Load config file.
		config.clear_config();

		std::filesystem::path config_path = _projects_path / std::filesystem::u8path(project_name + ".cbuild_config");
		std::hash<std::string> hash;
		config_path = _projects_path / std::filesystem::u8path(project_name + "_" + std::to_string(hash(config_path.string())) + ".cbuild_config");
		
		config.load_config(config_path);

		Config_Timestamps* timestamps = config.get_config_timestamps(_config_type);

		//Compile precompiled header.
		bool built_pch = false;
		u64 pch_time = 0;

		if (!precompiled_header.empty()) {

			bool found = false;

			if (File::file_exists(precompiled_header)) {
				found = true;
			}
			else {

				for (const std::filesystem::path& src_path : src_dirs) {

					std::filesystem::path pch_path = src_path / precompiled_header;
					File::format_path(pch_path);

					if (File::file_exists(pch_path)) {

						precompiled_header = pch_path;
						found = true;
						break;

					}

				}
				
			}

			if (!found) {
				CBUILD_WARN("Unable to locate precompiled header: '" + precompiled_header.string() + "'");
			}
			else {
				
				u64 time = std::filesystem::last_write_time(precompiled_header).time_since_epoch().count();

				if (_force_rebuild || !File::file_exists(std::filesystem::path(precompiled_header).replace_extension(".gch"))) built_pch = true;
				else if (config.last_used_type != _config_type || config.last_used_compiler != _compiler) {
					built_pch = true;
				}
				else {
					
					if (timestamps != nullptr && timestamps->timestamps.find(precompiled_header.string()) != timestamps->timestamps.end()) {
						
						u64 old_time = timestamps->timestamps[precompiled_header.string()];
						if (old_time != time) built_pch = true;

					}
					else {
						built_pch = true;
					}

				}

				pch_time = time;

				if (built_pch) {
					
					cmd = compiler->build_pch_cmd(precompiled_header, _config_type, *this);

					CBUILD_TRACE("Compiling PCH '{}'", precompiled_header.string());

					if (_print_cmds) CBUILD_TRACE(cmd);
					
					if (system(cmd.c_str()) != 0) { //@TODO: Check if returned with warning?

						CBUILD_ERROR("An error occurred while compiling precompiled header.");
						return false;

					}

				}

			}

		}
		
		//Compile source files.
		std::vector<std::filesystem::path> src_dir_files;
		std::vector<std::filesystem::path> obj_files;

		for (const std::filesystem::path& src_path : src_dirs) {
			
			if (!File::directory_exists(src_path)) {

				CBUILD_ERROR("Directory '" + src_path.string() + "' does not exist.");
				return false;

			}
			
			if (!File::find_files(src_path, ".c", src_dir_files)) continue;
			
			for (const std::filesystem::path& file : src_dir_files) {

				std::filesystem::path obj_path = obj_output_path / file.filename().replace_extension(".o");
				obj_files.push_back(obj_path);

				bool built = parse_source_and_header_files(file, _config_type, _compiler);
				if (!built && !_force_rebuild) continue;

				cmd = compiler->build_source_cmd(file, _config_type, *this);

				CBUILD_TRACE("Compiling '{}'", file.string());

				if (_print_cmds) CBUILD_TRACE(cmd);
				if (system(cmd.c_str()) != 0) { //@TODO: Check if returned with warning?

					CBUILD_ERROR("An error occurred.");
					config.save_config(config_path);

					return false;

				}

				built_something = true;

			}

		}

		for (const std::filesystem::path& src_file : src_files) {

			std::filesystem::path obj_path = obj_output_path / src_file.filename().replace_extension(".o");
			obj_files.push_back(obj_path);

			bool built = parse_source_and_header_files(src_file, _config_type, _compiler);
			if (!built && !_force_rebuild) continue;

			cmd = compiler->build_source_cmd(src_file, _config_type, *this);

			CBUILD_TRACE("Compiling '{}'", src_file.string());

			if (_print_cmds) CBUILD_TRACE(cmd);
			if (system(cmd.c_str()) != 0) {

				CBUILD_ERROR("An error occurred.");
				config.save_config(config_path);

				return false;

			}

			built_something = true;

		}

		config.last_used_type = _config_type;
		config.last_used_compiler = _compiler;

		for (const Checked_File& checked_file : checked_files) {
			config.set_config_timestamp(_config_type, checked_file.path, checked_file.time);
		}

		if (built_pch) {

			config.set_config_timestamp(_config_type, precompiled_header, pch_time);
			built_something = true;

		}

		if (!built_something) {
			CBUILD_TRACE("Everything is up-to-date.");
		}
		else {
			config.save_config(config_path);
		}

		//Generate static lib.
		if (build_type == Build_Type::Static_Lib) {

			std::string lib_name = "lib" + build_name + ".a";
			std::filesystem::path lib_path = build_output_path / std::filesystem::u8path(lib_name);
			if (!compiler->build_static_lib(lib_path, obj_files, _config_type, _print_cmds, *this)) return false;

		}

		//Generate binary.
		else if (build_type == Build_Type::Binary) {

			std::filesystem::path bin_path = build_output_path / std::filesystem::u8path(build_name);
			if (!compiler->build_binary(bin_path, obj_files, _config_type, _print_cmds, *this)) return false;

		}

		printf("");

		return true;

	}

}
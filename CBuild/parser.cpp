#include "pch.h"
#include "parser.h"
#include "lexer.h"
#include "log.h"
#include "file.h"

#include <filesystem>

#define COMMAND_FUNC(func) std::bind(&func, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

namespace CBuild {

	Parser::Parser() {

		cmds["set_compiler"]			= { COMMAND_FUNC(Parser::parse_cmd_set_compiler) };
		cmds["set_project_name"]		= { COMMAND_FUNC(Parser::parse_cmd_set_project_name) };
		cmds["set_build_type"]			= { COMMAND_FUNC(Parser::parse_cmd_set_build_type) };
		cmds["set_build_output"]		= { COMMAND_FUNC(Parser::parse_cmd_set_build_output) };
		cmds["set_build_name"]			= { COMMAND_FUNC(Parser::parse_cmd_set_build_name) };
		cmds["set_obj_output"]			= { COMMAND_FUNC(Parser::parse_cmd_set_obj_output) };
		cmds["set_precompiled_header"]	= { COMMAND_FUNC(Parser::parse_cmd_set_precompiled_header) };
		cmds["set_pch"]					= { COMMAND_FUNC(Parser::parse_cmd_set_precompiled_header) };
		cmds["set_run_exec"]			= { COMMAND_FUNC(Parser::parse_cmd_set_run_exec) };
		cmds["set_run_executable"]		= { COMMAND_FUNC(Parser::parse_cmd_set_run_exec) };
		cmds["add_src_dirs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_src_dirs) };
		cmds["add_incl_dirs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_incl_dirs) };
		cmds["add_lib_dirs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_lib_dirs) };
		cmds["add_static_libs"]			= { COMMAND_FUNC(Parser::parse_cmd_add_static_libs) };

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
			else if (token.type == Token_Type::String) {
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
		
		std::string compiler_name = _cur_token.value;
		String_Helper::lower(compiler_name);

		if (compiler_name != "gcc") {

			std::string msg = "Invalid compiler '" + _cur_token.value + "' specified in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		compiler_type = Compiler_Type::GCC;

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

		if (build_type_name != "executable" && build_type_name != "exec" && build_type_name != "static_lib") {

			std::string msg = "Invalid build_type '" + _cur_token.value + "' specified in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (build_type_name == "executable" || build_type_name == "exec") build_type = Build_Type::Executable;
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

		project_name = _cur_token.value;

		for (const char& c : project_name) {
			if (!lexer->is_valid_path_char(c)) {

				project_name = "";

				std::string msg = "Invalid project_name '" + _cur_token.value + "' specified in command '" + _prev_token.value + "'";
				error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);

				return false;

			}
		}

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_obj_output(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'obj_output_dir' in command '" + _prev_token.value + "'";
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

		build_name = _cur_token.value;

		for (const char& c : build_name) {
			if (!lexer->is_valid_path_char(c)) {

				build_name = "";

				std::string msg = "Invalid build_name '" + _cur_token.value + "' specified in command '" + _prev_token.value + "'";
				error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
				

				return false;

			}
		}

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_precompiled_header(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'precompiled_header' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		precompiled_header = std::filesystem::u8path(_cur_token.value);
		File::format_path(precompiled_header);

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_set_run_exec(u64& _index, Token& _cur_token, Token& _prev_token) {

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected argument 'run_exec' in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		std::string run = _cur_token.value;
		String_Helper::lower(run);

		if (run == "true") run_exec = true;
		else run_exec = false;

		return parse_semicolon(_index, _cur_token, _prev_token);

	}

	bool Parser::parse_cmd_add_src_dirs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_dirs(_index, _cur_token, _prev_token, src_dirs);
	}

	bool Parser::parse_cmd_add_incl_dirs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_dirs(_index, _cur_token, _prev_token, incl_dirs);
	}

	bool Parser::parse_cmd_add_lib_dirs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_dirs(_index, _cur_token, _prev_token, lib_dirs);
	}

	bool Parser::parse_cmd_add_static_libs(u64& _index, Token& _cur_token, Token& _prev_token) {
		return parse_cmd_add_strings(_index, _cur_token, _prev_token, static_libs);
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

	bool Parser::parse_cmd_add_strings(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::string>& _strings) {

		Token cmd_token = _cur_token;

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected at least 1 'string' argument in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		while (_cur_token.type == Token_Type::String) {

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

	void Parser::parse_timestamps(std::string& _data) {

		_data += "\n";

		u8 state = 0;
		std::string token;

		std::filesystem::path path;

		for (const char& c : _data) {

			//File name.
			if (state == 0) {

				if (c == '\n' || c == '\r' || c == '\t') {
					token = "";
				}
				else if (c == ' ') {
					
					if (!token.empty()) {

						path = std::filesystem::u8path(token);
						File::format_path(path);

						if (!File::file_exists(path)) {
							state = 0;
						}
						else {
							state = 1;
						}

						token = "";

					}

				}
				else {
					token += c;
				}

			}

			//Timestamp.
			else if (state == 1) {

				if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {

					if (!token.empty()) {

						std::stringstream stream(token);
						u64 time;
						stream >> time;

						timestamps[path.string()] = time;

					}

					token = "";
					state = 0;

				}
				else if (!lexer->is_digit(c)) {

					token = "";
					state = 0;

				}
				else {
					token += c;
				}

			}

		}

	}

	void Parser::write_timestamps(const std::filesystem::path& _path) {

		std::string text = "";

		auto it = timestamps.begin();
		while (it != timestamps.end()) {

			text += it->first + " " + std::to_string(it->second) + "\n";
			++it;

		}

		File::write_text_file(_path, text);

	}

	bool Parser::parse_include_directives(const std::filesystem::path& _path) {

		//@TODO: Standard libraries?

		//Check if the file has already been parsed.
		for (const Checked_File& checked_file : checked_files) {

			if (File::compare(checked_file.path, _path)) {
				return checked_file.rebuild;
			}

		}

		bool should_rebuild = false;

		if (!File::file_exists(_path)) {

			checked_files.push_back({ _path, false, 0 });
			return false;

		}

		//Rebuild source file if object file does not exist.
		if (_path.extension().string() == ".c") {

			std::filesystem::path obj_file_path = obj_output / _path.filename().replace_extension(".o");//+ "/" + _path.stem().string() + ".o";
			File::format_path(obj_file_path);

			if (!File::file_exists(obj_file_path)) should_rebuild = true; //@TODO: TEST THIS

		}

		//Compare timestamps.
		//@TODO: Check checksum instead?
		u64 time = std::filesystem::last_write_time(_path).time_since_epoch().count();

		if (!should_rebuild) {

			if (timestamps.find(_path.string()) != timestamps.end()) {

				u64 old_time = timestamps[_path.string()];
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

		for (u64 i = 0; i < c_lexer.tokens.size(); ++i) {

			C_Token& token = c_lexer.tokens[i];
			if (!(token.type == C_Token_Type::Directive && token.value == "#include")) continue;

			++i;
			if (i >= c_lexer.tokens.size()) break;

			token = c_lexer.tokens[i];

			if (token.type == C_Token_Type::String) {
				local_files.push_back(token.value);
			}
			else if (token.type == C_Token_Type::Include_String) {
				include_files.push_back(token.value);
			}
			else if (token.type == C_Token_Type::Directive && token.value == "#include") {

				--i;
				continue;

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

			if (parse_include_directives(local_path)) should_rebuild = true;

		}

		std::string incl_path = "";
		for (const std::string& include_file : include_files) {

			for (const std::filesystem::path& include_dir : incl_dirs) {

				std::filesystem::path incl_path = include_dir / std::filesystem::u8path(include_file);
				File::format_path(incl_path);

				if (!File::file_exists(incl_path)) continue;
				if (parse_include_directives(incl_path)) should_rebuild = true;

			}

		}

		checked_files.push_back({ _path, should_rebuild, time });
		return should_rebuild;

	}

	std::string Parser::create_gcc_base_cmd() {

		std::string cmd = "gcc -Wall";

		for (const std::filesystem::path& incl_dir : incl_dirs) {
			cmd += " -I " + incl_dir.string();
		}

		for (const std::filesystem::path& lib_dir : lib_dirs) {
			cmd += " -L " + lib_dir.string();
		}

		if (static_libs.size() > 0) {

			cmd += " -static";

			for (const std::string static_lib : static_libs) {
				cmd += " -l " + static_lib;
			}

		}

		return cmd;

	}

	std::string Parser::create_gcc_build_source_cmd(const std::filesystem::path& _source_file) {

		std::string cmd = create_gcc_base_cmd();

		std::filesystem::path obj_path = obj_output / _source_file.filename().replace_extension(".o");
		File::format_path(obj_path);

		cmd += " -c -o " + obj_path.string();

		return cmd;

	}

	std::string Parser::create_gcc_build_pch_cmd(const std::filesystem::path& _pch_file) {

		std::string cmd = create_gcc_base_cmd();

		std::filesystem::path gch_path = _pch_file;
		gch_path.replace_extension(".gch");

		cmd += " -c " + _pch_file.string() + " -o " + gch_path.string();

	}

	std::string Parser::create_gcc_build_static_lib_cmd(const std::filesystem::path& _lib_file, std::vector<std::filesystem::path>& _obj_files) {

		std::string name = "lib" + build_name + ".a";

		std::filesystem::path lib_path = build_output / std::filesystem::u8path(name);
		File::format_path(lib_path);

		std::string cmd = "ar rcs " + lib_path.string();

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " " + file.string();
			}

		}

		if (system(cmd.c_str()) != 0) {

			CBUILD_ERROR("Error occurred while linking static library.");
			return false;

		}

	}

	bool Parser::should_build() {
		return (src_dirs.size() > 0);
	}

	bool Parser::build(const std::filesystem::path& _projects_path, bool _force_rebuild) {

		if (compiler_type == Compiler_Type::GCC) {
			return build_gcc(_projects_path, _force_rebuild);
		}

		return true;

	}

	bool Parser::build_gcc(const std::filesystem::path& _projects_path, bool _force_rebuild) {

		//@TODO: Debug/release.
		//gcc -c src/thread.c -Wall -I . -I includes -L libraries/glfw3_x64/ -static -l glfw3 -l gdi32 -l winmm

		std::string cmd;

		if (!std::filesystem::is_directory(build_output)) {
			std::filesystem::create_directory(build_output);
		}

		if (!std::filesystem::is_directory(obj_output)) {
			std::filesystem::create_directory(obj_output);
		}

		bool built_something = false;

		std::filesystem::path timestamps_path = _projects_path / std::filesystem::u8path(project_name + ".cbuild_timestamp");
		File::format_path(timestamps_path);

		if (File::file_exists(timestamps_path)) {

			std::string data;
			if (File::read_text_file(timestamps_path, data)) {
				parse_timestamps(data);
			}

		}

		//Compile precompiled header.
		bool built_pch = false;
		u64 pch_time = 0;

		if (!precompiled_header.empty()) {

			bool found = false;

			for (const std::filesystem::path& src_path : src_dirs) {

				std::filesystem::path pch_path = src_path / precompiled_header;
				File::format_path(pch_path);

				if (File::file_exists(pch_path)) {

					precompiled_header = pch_path;
					found = true;
					break;

				}

			}

			if (!found) {
				CBUILD_WARN("Unable to locate precompiled header: '" + precompiled_header.string() + "'");
			}
			else {
				
				u64 time = std::filesystem::last_write_time(precompiled_header).time_since_epoch().count();

				if (_force_rebuild) built_pch = true;
				else {

					if (timestamps.find(precompiled_header.string()) != timestamps.end()) {

						u64 old_time = timestamps[precompiled_header.string()];
						if (old_time != time) built_pch = true;

					}
					else {
						built_pch = true;
					}

				}

				pch_time = time;

				if (built_pch) {

					cmd = create_gcc_build_pch_cmd(precompiled_header);

					CBUILD_TRACE("Compiling PCH '{}'", precompiled_header.string());
					if (system(cmd.c_str()) != 0) {

						CBUILD_ERROR("An error occurred while compiling precompiled header.");
						return 0;

					}

				}

			}

		}

		//Compile source files.
		std::vector<std::filesystem::path> src_files;
		std::vector<std::filesystem::path> obj_files;

		for (const std::filesystem::path& src_path : src_dirs) {

			if (!File::directory_exists(src_path)) {

				CBUILD_ERROR("Directory '" + src_path.string() + "' does not exist.");
				return false;

			}
			
			if (!File::find_files(src_path, ".c", src_files)) continue;

			for (const std::filesystem::path& file : src_files) {
				
				std::filesystem::path obj_path = obj_output / file.filename().replace_extension(".o");
				File::format_path(obj_path);
				obj_files.push_back(obj_path);

				bool built = parse_include_directives(file);
				if (!built && !_force_rebuild) continue;

				cmd = create_gcc_build_source_cmd(file);

				CBUILD_TRACE("Compiling '{}'", src_path.string() + "/" + file.filename().string());

				if (system(cmd.c_str()) != 0) {

					CBUILD_ERROR("An error occurred.");
					write_timestamps(timestamps_path);

					return false;

				}

				built_something = true;

			}

		}

		for (const Checked_File& checked_file : checked_files) {
			timestamps[checked_file.path.string()] = checked_file.time;
		}

		if (built_pch) {

			timestamps[precompiled_header] = pch_time;
			built_something = true;

		}

		if (!built_something) {
			CBUILD_TRACE("Everything is up-to-date.");
		}
		else {
			write_timestamps(timestamps_path);
		}

		//Generate static lib.
		if (build_type == Build_Type::Static_Lib) {

			cmd = create_gcc_build_static_lib_cmd(obj_files);

			if (system(cmd.c_str()) != 0) {

				CBUILD_ERROR("Error occurred while linking static library.");
				return false;

			}

			CBUILD_INFO("Generated '{}'", build_output + "/lib" + build_name + ".a");

		}

		//Generate executable.
		else if (build_type == Build_Type::Executable) {

			cmd = "gcc";

			for (const std::string& file : obj_files) {

				if (File::file_exists(file)) {
					cmd += " " + file;
				}

			}
			
			for (const std::string incl_dir : incl_dirs) {
				cmd += " -I " + incl_dir;
			}

			for (const std::string lib_dir : lib_dirs) {
				cmd += " -L " + lib_dir;
			}

			if (static_libs.size() > 0) {

				cmd += " -static";

				for (const std::string static_lib : static_libs) {
					cmd += " -l " + static_lib;
				}

			}

			cmd += " -o " + build_output + "/" + build_name;

			if (system(cmd.c_str()) != 0) {

				CBUILD_ERROR("Error occurred while linking executable.");
				return false;

			}

			CBUILD_INFO("Generated '{}'", build_output + "/" + build_name + ".exe");

			if (run_exec) {

				cmd = "cd " + build_output + " && \"" + build_name + "\"";
				system(cmd.c_str());

			}

		}

		return true;

	}

}
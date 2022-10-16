#include "pch.h"
#include "parser.h"
#include "lexer.h"
#include "log.h"
#include "file.h"

#include <filesystem>

namespace CBuild {

	Parser::Parser() {
		
		/*cmd_defs.push_back({"set_compiler", {"compiler_name"}});
		cmd_defs.push_back({ "set_build_type", { "build_type" } });
		cmd_defs.push_back({ "set_obj_output", { "obj_output_path" } });
		cmd_defs.push_back({ "set_build_output", 1 });
		cmd_defs.push_back({ "set_output_name", 1 });
		cmd_defs.push_back({ "set_output_name", 1 });
		cmd_defs.push_back({ "add_src_dirs", 0, -1 });
		cmd_defs.push_back({ "add_incl_dirs", 0, -1 });
		cmd_defs.push_back({ "add_lib_dirs", 0, -1 });
		cmd_defs.push_back({ "add_static_libs", 0, -1 });*/

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

	bool Parser::get_cmd_def(const std::string _name, Command_Def& _def) {

		for (Command_Def def : cmd_defs) {

			if (def.name == _name) {

				_def = def;
				return true;

			}

		}

		return false;

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

				std::string msg = "Unexpected string found: '" + token.value + "'";
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

		/*Command_Def def;
		if (!get_cmd_def(_cur_token.value, def)) {

			std::string msg = "Invalid command found: '" + _cur_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}*/

		if (!lexer->is_command(_cur_token.value)) { //This shouldn't ever happen.

			std::string msg = "Invalid command found: '" + _cur_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		if (_cur_token.value == "set_compiler") {
			if (!parse_cmd_set_compiler(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "set_build_type") {
			if (!parse_cmd_set_build_type(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "set_project_name") {
			if (!parse_cmd_set_project_name(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "set_obj_output") {
			if (!parse_cmd_set_obj_output(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "set_build_output") {
			if (!parse_cmd_set_build_output(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "set_build_name") {
			if (!parse_cmd_set_build_name(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "set_precompiled_header" || _cur_token.value == "set_pch") {
			if (!parse_cmd_set_precompiled_header(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "set_run_exec" || _cur_token.value == "set_run_executable") {
			if (!parse_cmd_set_run_exec(_index, _cur_token, _prev_token)) return false;
		}
		else if (_cur_token.value == "add_src_dirs") {
			if (!parse_cmd_add_dirs(_index, _cur_token, _prev_token, src_dirs)) return false;
		}
		else if (_cur_token.value == "add_incl_dirs") {
			if (!parse_cmd_add_dirs(_index, _cur_token, _prev_token, incl_dirs)) return false;
		}
		else if (_cur_token.value == "add_lib_dirs") {
			if (!parse_cmd_add_dirs(_index, _cur_token, _prev_token, lib_dirs)) return false;
		}
		else if (_cur_token.value == "add_static_libs") {
			if (!parse_cmd_add_strings(_index, _cur_token, _prev_token, static_libs)) return false;
		}

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

		obj_output = _cur_token.value;
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

		build_output = _cur_token.value;
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

		precompiled_header = _cur_token.value;
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

	bool Parser::parse_cmd_add_dirs(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::string>& _dirs) {

		Token cmd_token = _cur_token;

		get_next_token(_index, _cur_token, _prev_token);

		if (_cur_token.type != Token_Type::String) {

			std::string msg = "Expected at least 1 'dir' argument in command '" + _prev_token.value + "'";
			error_handler.set_error(Error_Type::Error, msg, _cur_token.line_pos, _cur_token.char_pos);
			return false;

		}

		while (_cur_token.type == Token_Type::String) {

			bool exists = false;

			std::string path_str = _cur_token.value;
			File::trim_path(path_str);
			if (path_str.empty()) path_str = "./";

			std::filesystem::path path = std::filesystem::u8path(path_str);
			for (const std::string& str : _dirs) {
				
				if (path == std::filesystem::u8path(str)) {

					exists = true;
					break;

				}

			}

			if (exists) {

				std::string msg = "Directory '" + _cur_token.value + "' has already been added in command '" + cmd_token.value + "'";
				error_handler.warn(msg, _cur_token.line_pos, _cur_token.char_pos);

			}
			else {
				_dirs.push_back(path_str);
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
		std::string token, file_name;

		for (const char& c : _data) {

			//File name.
			if (state == 0) {

				if (c == '\n' || c == '\r' || c == '\t') {
					token = "";
				}
				else if (c == ' ') {
					
					if (!token.empty()) {

						if (!File::file_exists(token)) {
							state = 0;
						}
						else {

							file_name = token;
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

						timestamps[file_name] = time;

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

	void Parser::write_timestamps(const std::string& _path) {

		std::string text = "";

		auto it = timestamps.begin();
		while (it != timestamps.end()) {

			text += it->first + " " + std::to_string(it->second) + "\n";
			++it;

		}

		File::write_text_file(_path, text);

	}

	bool Parser::parse_include_directives(const std::string& _path, s32 _tab) {

		//@TODO: Standard libraries?

		//Check if the file has already been parsed.
		std::filesystem::path path = std::filesystem::u8path(_path);
		std::string path_str = path.string(); //Just for consistency.
		
		for (const Checked_File& checked_file : checked_files) {

			if (checked_file.path.compare(path) == 0) {
				return checked_file.rebuild;
			}

		}

		bool should_rebuild = false;

		if (!File::file_exists(_path)) {

			checked_files.push_back({ path, false, 0 });
			return false;


		}

		//Rebuild source file if object file does not exist.
		if (path.extension().string() == ".c") {

			std::string obj_file_path = obj_output + "/" + path.stem().string() + ".o";
			File::format_path(obj_file_path);

			if (!File::file_exists(obj_file_path)) should_rebuild = true;

		}

		//Compare timestamps.
		//@TODO: Check checksum instead?
		u64 time = std::filesystem::last_write_time(path).time_since_epoch().count();

		if (!should_rebuild) {

			if (timestamps.find(path_str) != timestamps.end()) {

				u64 old_time = timestamps[path_str];
				if (old_time != time) should_rebuild = true;

			}
			else {
				should_rebuild = true;
			}

		}

		//Parse include directives in file.
		std::string source;
		if (!File::read_text_file(_path, source)) {

			checked_files.push_back({ path, false, time });
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

		}

		for (const std::string& local_file : local_files) {

			std::string local_path = path.has_parent_path() ? path.parent_path().string() + "/" + local_file : local_file;
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

			if (parse_include_directives(local_path, _tab + 1)) should_rebuild = true;

		}

		std::string incl_path = "";
		for (const std::string& include_files : include_files) {

			for (const std::string& include_dir : incl_dirs) {

				incl_path = include_dir + "/" + include_files;
				File::format_path(incl_path);

				if (!File::file_exists(incl_path)) continue;
				if (parse_include_directives(incl_path, _tab + 1)) should_rebuild = true;

			}

		}

		checked_files.push_back({ path, should_rebuild, time });
		return should_rebuild;

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
		std::vector<std::filesystem::path> files;

		std::string timestamps_path = _projects_path.string() + "/" + project_name + ".cbuild_timestamp";

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

			for (const std::string& src : src_dirs) {
				if (File::file_exists(src + "/" + precompiled_header)) {

					precompiled_header = src + "/" + precompiled_header;
					found = true;
					break;

				}
			}

			if (!found) {
				CBUILD_WARN("Unable to locate precompiled header: '" + precompiled_header + "'");
			}
			else {

				File::format_path(precompiled_header);
				std::filesystem::path pch_path = std::filesystem::u8path(precompiled_header);
				
				u64 time = std::filesystem::last_write_time(pch_path).time_since_epoch().count();

				if (_force_rebuild) built_pch = true;
				else {

					if (timestamps.find(precompiled_header) != timestamps.end()) {

						u64 old_time = timestamps[precompiled_header];
						if (old_time != time) built_pch = true;

					}
					else {
						built_pch = true;
					}

				}

				pch_time = time;

				if (built_pch) {

					cmd = "gcc -c -Wall";

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

					cmd += " " + precompiled_header;
					cmd += " -o " + precompiled_header + ".gch";

					CBUILD_TRACE("Compiling PCH '{}'", precompiled_header);
					if (system(cmd.c_str()) != 0) {

						CBUILD_ERROR("An error occurred while compiling precompiled header.");
						return 0;

					}

				}

			}

		}

		//Compile source files.
		std::vector<std::string> obj_files;
		for (const std::string dir : src_dirs) {

			if (!std::filesystem::is_directory(std::filesystem::u8path(dir))) {

				CBUILD_ERROR("Directory '" + dir + "' does not exist.");
				return false;

			}
			
			if (!File::find_files(dir, ".c", files)) continue;

			for (const std::filesystem::path& file : files) {
				
				std::string obj_path = obj_output + "/" + file.stem().string() + ".o";
				File::format_path(obj_path);
				obj_files.push_back(obj_path);

				std::string file_path = file.string();
				File::format_path(file_path);

				bool built = parse_include_directives(file_path, 0);
				if (!built && !_force_rebuild) continue;

				//Compile source file.
				cmd = "gcc " + file_path + " -c -Wall";

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

				cmd += " -o " + obj_output + "/" + file.stem().string() + ".o";
				CBUILD_TRACE("Compiling '{}'", dir + "/" + file.filename().string());

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
			CBUILD_TRACE("Everything is up to date.");
		}
		else {
			write_timestamps(timestamps_path);
		}

		//Generate static lib.
		if (build_type == Build_Type::Static_Lib) {

			cmd = "ar rcs " + build_output + "/" + build_name + ".a";

			for (const std::string& file : obj_files) {
				
				if (File::file_exists(file)) {
					cmd += " " + file;
				}

			}

			if (system(cmd.c_str()) != 0) {

				CBUILD_ERROR("Error occurred while linking static library.");
				return false;

			}

			CBUILD_INFO("Generated '{}'", build_output + "/" + build_name + ".a");

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

				cmd = "\"" + build_output + "\\" + build_name + "\"";
				system(cmd.c_str());

			}

		}

		return true;

	}

}
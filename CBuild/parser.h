#pragma once

#include <vector>
#include <filesystem>
#include <sstream>
#include <unordered_map>

#include "types.h"
#include "error_handler.h"
#include "c_lexer.h"
#include "string_helper.h"
#include "config.h"

namespace CBuild {

	struct Lexer;
	struct Token;

	enum class Compiler_Type : u8 {

		GCC,
		Clang,

	};

	enum class Build_Type: u8 {

		Executable,
		Static_Lib,

	};

	struct Command {

		std::function<bool(u64&, Token&, Token&)> callback;

	};

	struct Checked_File {

		std::filesystem::path path;
		bool rebuild = false;
		u64 time = 0;

	};

	struct Parser {

		Error_Handler error_handler;
		Lexer* lexer = nullptr;
		C_Lexer c_lexer;
		Config config;

		std::unordered_map<std::string, Command> cmds;

		Compiler_Type compiler_type = Compiler_Type::GCC;
		Build_Type build_type = Build_Type::Executable;

		std::string project_name = "";
		std::filesystem::path precompiled_header = "";
		std::filesystem::path obj_output;
		std::filesystem::path build_output;
		std::string build_name = "";

		std::vector<std::filesystem::path> src_dirs;
		std::vector<std::filesystem::path> src_files;
		std::vector<std::filesystem::path> incl_dirs;
		std::vector<std::filesystem::path> lib_dirs;
		std::vector<std::string> static_libs;

		bool run_exec = false;

		std::vector<Checked_File> checked_files;

		Parser();

		bool get_current_token(const u64 _index, Token& _cur_token);
		bool get_next_token(u64& _index, Token& _cur_token, Token& _prev_token);
		bool get_prev_token(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_tokens(Lexer* lexer);
		bool parse_semicolon(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_compiler(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_build_type(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_project_name(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_obj_output(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_build_output(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_build_name(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_precompiled_header(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_set_run_exec(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_add_src_dirs(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_add_src_files(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_add_incl_dirs(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_add_lib_dirs(u64& _index, Token& _cur_token, Token& _prev_token);
		bool parse_cmd_add_static_libs(u64& _index, Token& _cur_token, Token& _prev_token);

		bool parse_cmd_add_dirs(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::filesystem::path>& _dirs);
		bool parse_cmd_add_files(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::filesystem::path>& _files);
		bool parse_cmd_add_strings(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::string>& _strings);

		bool parse_source_and_header_files(const std::filesystem::path& _path, Config_Type _config_type);

		std::filesystem::path get_obj_output_path(Config_Type _config_type);
		std::filesystem::path get_build_output_path(Config_Type _config_type);

		std::string create_gcc_clang_base_args(Compiler_Type _compiler, Config_Type _config_type);
		std::string create_gcc_clang_build_source_cmd(Compiler_Type _compiler, const std::filesystem::path& _source_file, Config_Type _config_type);
		std::string create_gcc_clang_build_pch_cmd(Compiler_Type _compiler, const std::filesystem::path& _pch_file, Config_Type _config_type);
		std::string create_gcc_clang_build_exec_cmd(Compiler_Type _compiler, const std::filesystem::path& _exec_file, std::vector<std::filesystem::path>& _obj_files, Config_Type _config_type);
		std::string create_gcc_clang_build_static_lib_cmd(Compiler_Type _compiler, const std::filesystem::path& _lib_file, std::vector<std::filesystem::path>& _obj_files);

		bool should_build();
		bool build(const std::filesystem::path& _projects_path, bool _force_rebuild = false, bool _print_cmds = false, Config_Type _config_type = Config_Type::Debug);
		bool build_gcc_clang(Compiler_Type _compiler, const std::filesystem::path& _projects_path, bool _force_rebuild = false, bool _print_cmds = false, Config_Type _config_type = Config_Type::Debug);

	};

}

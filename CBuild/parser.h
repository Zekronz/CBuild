#pragma once

#include <vector>
#include <filesystem>
#include <map>
#include <sstream>

#include "types.h"
#include "error_handler.h"
#include "c_lexer.h"
#include "string_helper.h"

namespace CBuild {

	struct Lexer;
	struct Token;

	enum class Compiler_Type : uint8_t {

		GCC,

	};

	enum class Build_Type: uint8_t {

		Executable,
		Static_Lib,

	};

	struct Command_Def {

		std::string name = "";
		std::vector<std::string> required_args;
		std::vector<std::string> optional_args;
		bool allow_unlimited_optional_args = false;

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

		std::vector<Command_Def> cmd_defs;

		Compiler_Type compiler_type = Compiler_Type::GCC;
		Build_Type build_type = Build_Type::Executable;

		std::string project_name = "";
		std::string obj_output = "";
		std::string build_output = "";
		std::string build_name = "";
		std::string precompiled_header = "";

		std::vector<std::string> src_dirs;
		std::vector<std::string> incl_dirs;
		std::vector<std::string> lib_dirs;
		std::vector<std::string> static_libs;

		bool run_exec = false;

		std::map<std::string, u64> timestamps;
		std::vector<Checked_File> checked_files;

		Parser();

		bool get_current_token(const u64 _index, Token& _cur_token);
		bool get_next_token(u64& _index, Token& _cur_token, Token& _prev_token);
		bool get_prev_token(u64& _index, Token& _cur_token, Token& _prev_token);
		bool get_cmd_def(const std::string _name, Command_Def& _def);
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
		bool parse_cmd_add_dirs(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::string>& _dirs);
		bool parse_cmd_add_strings(u64& _index, Token& _cur_token, Token& _prev_token, std::vector<std::string>& _strings);

		void parse_timestamps(std::string& _data);
		void write_timestamps(const std::string& _path);

		bool parse_include_directives(const std::string& _path, s32 _tab);

		bool should_build();
		bool build(const std::filesystem::path& _projects_path, bool _force_rebuild = false);
		bool build_gcc(const std::filesystem::path& _projects_path, bool _force_rebuild = false);

	};

}

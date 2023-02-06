#include "log.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"
#include "string_helper.h"

#include <Windows.h>

using namespace CBuild;

int main(int argc, char** argv) {
	
	//@TODO: Check timestamp of build file?

	Log::init();

	//Read input file and flags.
	std::vector<std::string> flags;
	std::string flag;
	std::string input_file = "";

	bool flag_force_rebuild = false;
	bool flag_print_cmds = false;
	Config_Type config_type = Config_Type::Debug;

	for (int i = 1; i < argc; ++i) {

		flag = argv[i];
		String_Helper::trim(flag);

		if (flag.length() > 0 && flag[0] != '-') {

			if(input_file.empty()) input_file = flag;
			continue;

		}

		if (flag.length() >= 2 && flag[0] == '-' && std::find(flags.begin(), flags.end(), flag) == flags.end()) {

			flags.push_back(flag);
			if (flag == "-force_rebuild" || flag == "-fr") flag_force_rebuild = true;
			else if (flag == "-pcmds") flag_print_cmds = true;
			else if (flag == "-release") config_type = Config_Type::Release;
			else CBUILD_WARN("Unknown flag '{}' found.", flag);

		}

	}

	if (input_file.empty()) {

		CBUILD_ERROR("No input file specified.");
		return 1;

	}

	std::filesystem::path input_file_path = std::filesystem::u8path(input_file);
	File::format_path(input_file_path);
	if (!input_file_path.has_extension()) input_file_path.replace_extension(".cbuild");

	if (!File::file_exists(input_file_path)) {

		CBUILD_ERROR("The specified input file does not exist.");
		return 1;

	}
	
	std::string source;
	if (!File::read_text_file(input_file_path, source)) {

		CBUILD_ERROR("Error reading input file.");
		return 1;

	}
	
	//Parse input file.
	Lexer lexer;
	lexer.parse_source(source);

	if (lexer.error_handler.has_error()) {

		CBUILD_ERROR(lexer.error_handler.get_error_message());
		return 1;

	}
	
	Parser parser;
	parser.parse_tokens(&lexer);

	if (parser.error_handler.has_error()) {

		CBUILD_ERROR(parser.error_handler.get_error_message());
		return 1;

	}
	
	if (!parser.should_build()) {

		CBUILD_TRACE("Nothing to build.");
		return 1;

	}
	
	//Create timestamps folder.
	char* exec_dir = argv[0];
	std::filesystem::path exec_path = std::filesystem::u8path(exec_dir);

	char buffer[1024];
	DWORD result = GetModuleFileNameA(NULL, &buffer[0], 1024); //Gets the absolute executable path of CBuild.
	if (result != 0 && result < 1024)  exec_path = std::filesystem::u8path(buffer);

	exec_path = exec_path.has_parent_path() ? exec_path.parent_path() : "";
	File::format_path(exec_path);
	
	std::filesystem::path projects_path = exec_path / std::filesystem::u8path("configs");
	
	if (!std::filesystem::is_directory(projects_path)) {
		std::filesystem::create_directory(projects_path);
	}

	//Build.
	if (!parser.build(projects_path, flag_force_rebuild, flag_print_cmds, config_type)) {
		return 1;
	}

	return 0;

}
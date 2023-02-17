#include "pch.h"
#include "compiler_spec.h"
#include "parser.h"

namespace CBuild {

	Compiler_Spec::Compiler_Spec(Compiler_Type _type, const std::string _name, const std::string _archiver_name) : type(_type), name(_name), archiver_name(_archiver_name) {}

	std::string Compiler_Spec::init_cmd(const std::string& _name, Parser& _parser) {
		return "\"" + _parser.get_compiler_path(_name).string() + "\"";
	}

	void Compiler_Spec::add_includes_and_libraries(std::string& _cmd, Parser& _parser) {

		for (const std::filesystem::path& incl_dir : _parser.incl_dirs) {
			_cmd += " -I \"" + incl_dir.string() + "\"";
		}

		for (const std::filesystem::path& lib_dir : _parser.lib_dirs) {
			_cmd += " -L \"" + lib_dir.string() + "\"";
		}

		if (_parser.static_libs.size() > 0) {

			_cmd += " -static";

			for (const std::string static_lib : _parser.static_libs) {
				_cmd += " -l " + static_lib;
			}

		}

	}

	//GCC.
	Compiler_Spec_GCC::Compiler_Spec_GCC() : Compiler_Spec(Compiler_Type::GCC, "gcc", "ar") {}

	void Compiler_Spec_GCC::add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) {

		if (_config == Config_Type::Debug)	_cmd += " -Wall -g";
		else								_cmd += " -Wall -O3";

	}

	std::string Compiler_Spec_GCC::build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);

		add_common_flags(cmd, _config, _parser);
		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path obj_path = _parser.get_obj_output_path(_config) / _source.filename().replace_extension(".o");
		File::format_path(obj_path);

		cmd += " -c -o \"" + obj_path.string() + "\"";
		cmd += " \"" + _source.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_GCC::build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);
		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path gch_path = _pch;
		gch_path.replace_extension(".gch");

		cmd += " -c \"" + _pch.string() + "\" -o \"" + gch_path.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_GCC::build_binary_cmd(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		add_includes_and_libraries(cmd, _parser);

		cmd += " -o \"" + _binary.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_GCC::build_static_lib_cmd(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(archiver_name, _parser);
		cmd += " rcs \"" + _lib.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		return "\"" + cmd + "\"";

	}
	
	//AVR-GCC.
	Compiler_Spec_AVR_GCC::Compiler_Spec_AVR_GCC() : Compiler_Spec(Compiler_Type::AVR_GCC, "avr-gcc", "avr-ar") {}

	void Compiler_Spec_AVR_GCC::add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) {

		//_cmd += " -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I \"" + _parser.get_atmel_studio_include_path().string() + "\" -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\" -c -std=gnu99";
		//@TODO: Release

	}

	std::string Compiler_Spec_AVR_GCC::build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);
		cmd += " -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I \"" + _parser.get_atmel_studio_include_path().string() + "\" -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\" -c -std=gnu99";

		add_common_flags(cmd, _config, _parser);
		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path obj_path = _parser.get_obj_output_path(_config) / _source.filename().replace_extension(".o");
		File::format_path(obj_path);

		std::filesystem::path d_path = obj_path.replace_extension(".d");

		cmd += " -MD -MP -MF \"" + d_path.string() + "\" -MT \"" + d_path.string() + "\" -MT \"" + obj_path.string() + "\" -o \"" + obj_path.string() + "\"";
		cmd += " \"" + _source.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_AVR_GCC::build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		cmd += " -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I \"" + _parser.get_atmel_studio_include_path().string() + "\" -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\" -c -std=gnu99";

		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path gch_path = _pch;
		gch_path.replace_extension(".gch");

		cmd += " \"" + _pch.string() + "\" -o \"" + gch_path.string() + "\"";

		//@TODO: Test this

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_AVR_GCC::build_binary_cmd(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, Parser& _parser) {
		
		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		std::filesystem::path elf_path = _binary;
		elf_path = elf_path.replace_extension(".elf");

		cmd += " -o \"" + elf_path.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		cmd += " -Wl,-Map=\"" + elf_path.replace_extension(".map").string() + "\" -Wl,--start-group -Wl,-lm -Wl,--end-group -Wl,--gc-sections -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\"";

		add_includes_and_libraries(cmd, _parser);

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_AVR_GCC::build_static_lib_cmd(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(archiver_name, _parser);
		cmd += " rcs \"" + _lib.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		return "\"" + cmd + "\"";

	}

	//Clang.
	void Compiler_Spec_Clang::add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) {

		if (_config == Config_Type::Debug)	_cmd += " -Wall -g";
		else								_cmd += " -Wall -O3";

	}

	Compiler_Spec_Clang::Compiler_Spec_Clang() : Compiler_Spec(Compiler_Type::Clang, "clang", "llvm-ar") {}

	std::string Compiler_Spec_Clang::build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);

		add_common_flags(cmd, _config, _parser);
		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path obj_path = _parser.get_obj_output_path(_config) / _source.filename().replace_extension(".o");
		File::format_path(obj_path);

		cmd += " -c -o \"" + obj_path.string() + "\"";
		cmd += " \"" + _source.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_Clang::build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);
		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path gch_path = _pch;
		gch_path.replace_extension(".gch");

		cmd += " -c \"" + _pch.string() + "\" -o \"" + gch_path.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_Clang::build_binary_cmd(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, Parser& _parser) {
		
		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		add_includes_and_libraries(cmd, _parser);

		cmd += " -o \"" + _binary.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_Clang::build_static_lib_cmd(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(archiver_name, _parser);
		cmd += " rcs \"" + _lib.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		return "\"" + cmd + "\"";

	}

}
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

		if (_config == Config_Type::Debug)	_cmd += " -Wall -g -D DEBUG";
		else								_cmd += " -Wall -O3 -D NDEBUG";

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

		std::filesystem::path gch_path = std::filesystem::path(_pch).replace_extension(".gch");

		cmd += " -c \"" + _pch.string() + "\" -o \"" + gch_path.string() + "\"";

		return "\"" + cmd + "\"";

	}

	bool Compiler_Spec_GCC::build_binary(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		add_includes_and_libraries(cmd, _parser);

		cmd += " -o \"" + _binary.string() + "\"";
		cmd = "\"" + cmd + "\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {

			CBUILD_ERROR("Error occurred while linking binary.");
			return false;

		}

		CBUILD_INFO("Generated '{}'", _binary.string() + ".exe");

		if (_parser.run_binary) {

			cmd = "cd " + _parser.get_build_output_path(_config).string() + " && \"" + _parser.build_name + "\"";

			if (_print_cmds) CBUILD_TRACE(cmd);
			system(cmd.c_str());

		}

		return true;

	}

	bool Compiler_Spec_GCC::build_static_lib(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) {

		std::string cmd = init_cmd(archiver_name, _parser);
		cmd += " rcs \"" + _lib.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		cmd = "\"" + cmd + "\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {

			CBUILD_ERROR("Error occurred while linking static library.");
			return false;

		}

		CBUILD_INFO("Generated '{}'", _lib.string());

		return true;

	}
	
	//AVR-GCC.
	Compiler_Spec_AVR_GCC::Compiler_Spec_AVR_GCC() : Compiler_Spec(Compiler_Type::AVR_GCC, "avr-gcc", "avr-ar") {}

	void Compiler_Spec_AVR_GCC::add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) {

	}

	std::string Compiler_Spec_AVR_GCC::build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);

		if (_config == Config_Type::Release) {
			cmd += " -x c -funsigned-char -funsigned-bitfields -DNDEBUG  -I \"" + _parser.get_atmel_studio_include_path().string() + "\" -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\" -c -std=gnu99";
		}
		else {
			cmd += " -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I \"" + _parser.get_atmel_studio_include_path().string() + "\" -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\" -c -std=gnu99";
		}

		add_common_flags(cmd, _config, _parser);
		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path obj_path = _parser.get_obj_output_path(_config) / _source.filename().replace_extension(".o");
		File::format_path(obj_path);

		std::filesystem::path d_path = std::filesystem::path(obj_path).replace_extension(".d");

		cmd += " -MD -MP -MF \"" + d_path.string() + "\" -MT \"" + d_path.string() + "\" -MT \"" + obj_path.string() + "\" -o \"" + obj_path.string() + "\"";
		cmd += " \"" + _source.string() + "\"";

		return "\"" + cmd + "\"";

	}

	std::string Compiler_Spec_AVR_GCC::build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) {

		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		if (_config == Config_Type::Release) {
			cmd += " -x c -funsigned-char -funsigned-bitfields -DNDEBUG  -I \"" + _parser.get_atmel_studio_include_path().string() + "\" -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\" -c -std=gnu99";
		}
		else {
			cmd += " -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I \"" + _parser.get_atmel_studio_include_path().string() + "\" -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\" -c -std=gnu99";
		}

		add_includes_and_libraries(cmd, _parser);

		std::filesystem::path gch_path = std::filesystem::path(_pch).replace_extension(".gch");

		cmd += " \"" + _pch.string() + "\" -o \"" + gch_path.string() + "\"";

		return "\"" + cmd + "\"";

	}

	bool Compiler_Spec_AVR_GCC::build_binary(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) {
		
		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		std::filesystem::path elf_path = std::filesystem::path(_binary).replace_extension(".elf");
		std::filesystem::path map_path = std::filesystem::path(_binary).replace_extension(".map");
		std::filesystem::path hex_path = std::filesystem::path(_binary).replace_extension(".hex");
		std::filesystem::path eep_path = std::filesystem::path(_binary).replace_extension(".eep");

		cmd += " -o \"" + elf_path.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		cmd += " -Wl,-Map=\"" + map_path.string() + "\" -Wl,--start-group -Wl,-lm -Wl,--end-group -Wl,--gc-sections -mmcu=" + _parser.avr_mcu + " -B \"" + _parser.get_atmel_studio_mcu_path().string() + "\"";

		add_includes_and_libraries(cmd, _parser);

		cmd = "\"" + cmd + "\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {

			CBUILD_ERROR("Error occurred while linking binary.");
			return false;

		}

		CBUILD_INFO("Generated '{}'", _binary.string() + ".elf");

		cmd = "\"" + init_cmd("avr-size", _parser) + " \"" + elf_path.string() + "\"\"";
		system(cmd.c_str());

		cmd = "\"" + init_cmd("avr-objcopy", _parser) + " -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures \"" + elf_path.string() + "\" \"" + hex_path.string() + "\"\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {
			CBUILD_WARN("Error occurred while generating '{}'", hex_path.string());
		}
		else {

			CBUILD_INFO("Generated '{}'", hex_path.string());

			cmd = "\"" + init_cmd("avr-size", _parser) + " \"" + hex_path.string() + "\"\"";
			system(cmd.c_str());

		}

		cmd = "\"" + init_cmd("avr-objcopy", _parser) + " -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex \"" + elf_path.string() + "\" \"" + eep_path.string() + "\" || exit 0\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {
			CBUILD_WARN("Error occurred while generating '{}'", eep_path.string());
		}
		else {
			
			CBUILD_INFO("Generated '{}'", eep_path.string());

			cmd = "\"" + init_cmd("avr-size", _parser) + " \"" + eep_path.string() + "\"\"";
			system(cmd.c_str());

		}

		if (_parser.run_binary) {

			std::filesystem::path dfu_path = _parser.exec_path / std::filesystem::u8path("tools/dfu-programmer/dfu-programmer.exe");			
			File::format_path(dfu_path);

			if (!File::file_exists(dfu_path)) {

				CBUILD_WARN("Unable to locate: '{}'", dfu_path.string());
				return true;

			}

			cmd = "\"\"" + dfu_path.string() + "\" " + _parser.avr_mcu + " erase --force\"";
			if (_print_cmds) CBUILD_TRACE(cmd);
			if (system(cmd.c_str()) != 0) {

				CBUILD_WARN("Error occurred while uploading to device.");
				return true;

			}

			cmd = "\"\"" + dfu_path.string() + "\" " + _parser.avr_mcu + " flash \"" + hex_path.string() + "\"\"";
			if (_print_cmds) CBUILD_TRACE(cmd);
			if (system(cmd.c_str()) != 0) {

				CBUILD_WARN("Error occurred while uploading to device.");
				return true;

			}

			cmd = "\"\"" + dfu_path.string() + "\" " + _parser.avr_mcu + " reset\"";
			if (_print_cmds) CBUILD_TRACE(cmd);
			if (system(cmd.c_str()) != 0) {

				CBUILD_WARN("Error occurred while uploading to device.");
				return true;

			}

		}

		return true;

	}

	bool Compiler_Spec_AVR_GCC::build_static_lib(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) {

		std::string cmd = init_cmd(archiver_name, _parser);
		cmd += " rcs \"" + _lib.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		cmd = "\"" + cmd + "\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {

			CBUILD_ERROR("Error occurred while linking static library.");
			return false;

		}

		CBUILD_INFO("Generated '{}'", _lib.string());

		//@TODO: Test this.

		return true;

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

		std::filesystem::path gch_path = std::filesystem::path(_pch).replace_extension(".gch");

		cmd += " -c \"" + _pch.string() + "\" -o \"" + gch_path.string() + "\"";

		return "\"" + cmd + "\"";

	}

	bool Compiler_Spec_Clang::build_binary(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) {
		
		std::string cmd = init_cmd(name, _parser);
		add_common_flags(cmd, _config, _parser);

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		add_includes_and_libraries(cmd, _parser);

		cmd += " -o \"" + _binary.string() + "\"";
		cmd = "\"" + cmd + "\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {

			CBUILD_ERROR("Error occurred while linking binary.");
			return false;

		}

		CBUILD_INFO("Generated '{}'", _binary.string() + ".exe");

		if (_parser.run_binary) {

			cmd = "cd " + _parser.get_build_output_path(_config).string() + " && \"" + _parser.build_name + "\"";

			if (_print_cmds) CBUILD_TRACE(cmd);
			system(cmd.c_str());

		}

		return true;

	}

	bool Compiler_Spec_Clang::build_static_lib(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) {

		std::string cmd = init_cmd(archiver_name, _parser);
		cmd += " rcs \"" + _lib.string() + "\"";

		for (const std::filesystem::path& file : _obj_files) {

			if (File::file_exists(file)) {
				cmd += " \"" + file.string() + "\"";
			}

		}

		cmd = "\"" + cmd + "\"";

		if (_print_cmds) CBUILD_TRACE(cmd);
		if (system(cmd.c_str()) != 0) {

			CBUILD_ERROR("Error occurred while linking static library.");
			return false;

		}

		CBUILD_INFO("Generated '{}'", _lib.string());

		return true;

	}

}
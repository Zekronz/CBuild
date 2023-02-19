#pragma once

#include <filesystem>
#include <vector>
#include <string>

#include "types.h"
#include "config.h"

namespace CBuild {

	enum class Compiler_Type : u8 {

		Invalid,
		GCC,
		AVR_GCC,
		Clang,

	};

	struct Parser;

	struct Compiler_Spec {

		Compiler_Type type;
		std::string name = "";
		std::string archiver_name = "";

		Compiler_Spec(Compiler_Type _type, const std::string _name, const std::string _archiver_name);
		
		virtual void add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) = 0;
		virtual std::string build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) = 0;
		virtual std::string build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) = 0;
		virtual bool build_binary(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) = 0;
		virtual bool build_static_lib(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) = 0;

		std::string init_cmd(const std::string& _name, Parser& _parser);
		void add_includes_and_libraries(std::string& _cmd, Parser& _parser);

	};

	struct Compiler_Spec_GCC : Compiler_Spec {

		Compiler_Spec_GCC();

		void add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) override;
		std::string build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) override;
		std::string build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) override;
		bool build_binary(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) override;
		bool build_static_lib(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) override;

	};

	struct Compiler_Spec_AVR_GCC : Compiler_Spec {

		Compiler_Spec_AVR_GCC();
		
		void add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) override;
		std::string build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) override;
		std::string build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) override;
		bool build_binary(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) override;
		bool build_static_lib(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) override;

	};

	struct Compiler_Spec_Clang : Compiler_Spec {

		Compiler_Spec_Clang();

		void add_common_flags(std::string& _cmd, const Config_Type _config, Parser& _parser) override;
		std::string build_source_cmd(const std::filesystem::path _source, const Config_Type _config, Parser& _parser) override;
		std::string build_pch_cmd(const std::filesystem::path _pch, const Config_Type _config, Parser& _parser) override;
		bool build_binary(const std::filesystem::path _binary, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) override;
		bool build_static_lib(const std::filesystem::path _lib, std::vector<std::filesystem::path>& _obj_files, const Config_Type _config, bool _print_cmds, Parser& _parser) override;

	};

}
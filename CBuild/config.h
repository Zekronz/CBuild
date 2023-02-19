#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "types.h"
#include "file.h"
#include "string_helper.h"

namespace CBuild {

	enum class Config_Type : u8 {

		Invalid,
		Debug,
		Release,

	};

	struct Config_Timestamps {

		Config_Type type = Config_Type::Invalid;
		std::unordered_map<std::string, u64> timestamps;

	};

	struct Config {

		Config_Type last_used_type = Config_Type::Invalid;
		std::string last_used_compiler = "gcc";
		std::unordered_map<Config_Type, Config_Timestamps> configs;

		Config_Type string_to_config_type(std::string _config_name);
		std::string config_type_to_string(Config_Type _type);
		
		Config_Timestamps* get_config_timestamps(Config_Type _type);
		void set_config_timestamp(Config_Type _type, const std::filesystem::path& _path, u64 _time);
		void clear_config();
		bool load_config(std::filesystem::path _path);
		bool save_config(std::filesystem::path _path);

	};

}
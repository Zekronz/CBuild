#include "pch.h"
#include "config.h"

namespace CBuild {

	Config_Type Config::string_to_config_type(std::string _config_name) {

		String_Helper::lower(_config_name);

		if (_config_name == "debug") return Config_Type::Debug;
		if (_config_name == "release") return Config_Type::Release;

		return Config_Type::Invalid;

	}

	std::string Config::config_type_to_string(Config_Type _type) {

		if (_type == Config_Type::Debug) return "debug";
		if (_type == Config_Type::Release) return "release";

		return "invalid";

	}

	Config_Timestamps* Config::get_config_timestamps(Config_Type _type) {

		const auto& it = configs.find(_type);
		if (it == configs.end()) return nullptr;

		return &it->second;

	}

	void Config::set_config_timestamp(Config_Type _type, const std::filesystem::path& _path, u64 _time) {

		Config_Timestamps* timestamps = get_config_timestamps(_type);
		if (timestamps == nullptr) {

			Config_Timestamps t;
			t.type = _type;

			configs[_type] = t;
			timestamps = get_config_timestamps(_type);

		}

		timestamps->timestamps[_path.string()] = _time;

	}

	void Config::clear_config() {

		last_used_type = Config_Type::Invalid;
		configs.clear();

	}

	bool Config::load_config(std::filesystem::path _path) {

		clear_config();

		std::string source;
		if (!File::read_text_file(_path, source)) return false;
		source += '\n';

		u8 state = 0;
		u8 string_state = 0;
		std::string token;

		Config_Type config_type = Config_Type::Invalid;
		std::filesystem::path timestamp_path;

		for (const char& c : source) {

			//Config.
			if (state == 0) {

				if (c == '\n') {
					token = "";
				}
				else if (isspace(c)) {

					if (token.empty()) continue;
					
					if (token == "last_used_config_type") {

						token = "";
						state = 3;

						continue;

					}

					Config_Type t = string_to_config_type(token);
					if (t == Config_Type::Invalid) {

						token = "";
						state = 4;

						continue;

					}

					config_type = t;
					token = "";
					state = 1;
					string_state = 0;

				}
				else {
					token += c;
				}

			}

			//File name.
			else if (state == 1) {

				if (c == '\n') {

					token = "";
					state = 0;

				}
				else if (c == '"') {

					if (string_state == 0) {

						++string_state;
						continue;

					}

					++string_state;
					
					timestamp_path = std::filesystem::u8path(token);
					File::format_path(timestamp_path);

					if (!File::file_exists(timestamp_path)) {
						state = 4;
					}

					token = "";

				}
				else if (isspace(c) && (string_state == 0 || string_state == 2)) {

					if (string_state == 2) {
						state = 2;
					}

					continue;

				}
				else {

					if (string_state != 1) {

						token = "";
						state = 4;

						continue;
						
					}

					token += c;

				}

			}

			//Timestamp.
			else if (state == 2) {

				if (c == '\n' || (isspace(c) && !token.empty())) {

					if (!token.empty()) {

						std::stringstream stream(token);
						u64 time;
						stream >> time;

						set_config_timestamp(config_type, timestamp_path, time);

					}

					token = "";
					state = (c == '\n') ? 0 : 4;

				}
				else if (isspace(c)) {
					continue;
				}
				else if (c >= '0' && c <= '9') {
					token += c;

				}
				else {
					token = "";
					state = 4;
				}

			}

			//Last used config type.
			else if (state == 3) {

				if (c == '\n' || (isspace(c) && !token.empty())) {

					if (!token.empty()) {

						Config_Type t = string_to_config_type(token);
						if (t != Config_Type::Invalid) last_used_type = t;

					}

					token = "";
					state = (c == '\n') ? 0 : 4;

				}
				else if (isspace(c)) {
					continue;
				}
				else {
					token += c;
				}

			}

			//Wait for new line.
			else if (state == 4) {

				if (c == '\n') {
					state = 0;
				}

			}

		}

		return true;

	}

	bool Config::save_config(std::filesystem::path _path) {

		std::string source;

		if (last_used_type != Config_Type::Invalid) source += "last_used_config_type " + config_type_to_string(last_used_type) + "\n\n";

		size_t config_ind = 0;
		size_t config_count = configs.size();

		auto config_it = configs.begin();
		while (config_it != configs.end()) {

			Config_Timestamps t = config_it->second;

			auto timestamp_it = t.timestamps.begin();
			while (timestamp_it != t.timestamps.end()) {

				source += config_type_to_string(config_it->first) + " \"" + timestamp_it->first + "\" " + std::to_string(timestamp_it->second) + "\n";
				++timestamp_it;

			}

			if(config_ind < config_count - 1) source += "\n";

			++config_ind;
			++config_it;

		}

		return File::write_text_file(_path, source);

	}

}
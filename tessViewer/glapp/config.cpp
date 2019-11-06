#include "config.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace glapp {
using namespace nlohmann;
Config::Config(const std::filesystem::path& config_file) {
	try {
		std::ifstream ifs(config_file);
		ifs.exceptions(std::ios_base::failbit);
		json j;
		ifs >> j;
		_jconfig = std::move(j);
	}
	catch (std::exception& e) {
		std::cerr << e.what();
	}
}
Config::Config(const Config& config)     = default;
Config::Config(Config&& config) noexcept = default;
// : _jconfig(std::move(config._jconfig)) {}
Config::~Config()       = default;
Config& Config::operator=(const Config& config) {
	_jconfig = config._jconfig;
	return *this;
}
Config& Config::operator=(Config&& config) noexcept {
	if (this != &config) {
		_jconfig = std::move(config._jconfig);
	}
	return *this;
}
const json& Config::Json() const { return _jconfig; }

const Config Config::_config = Config("setting.json");
;
const json Config::_jschema = Config::_load_schema();
json       Config::_load_schema() {
#include "../doc/schemas/setting_schema.json.gen.h"
	return json::parse(setting_json);
}
} // namespace glapp

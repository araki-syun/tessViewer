#include "config.h"
#include "fmt/core.h"
#include "nlohmann/json_fwd.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <array>

#include "../log.h"

namespace glapp {
using namespace nlohmann;
Config::Config(const std::filesystem::path& config_file) {
	std::ifstream ifs(config_file);
	ifs.exceptions(std::ios_base::failbit);
	auto j = std::shared_ptr<json>();
	ifs >> *j;
	_jconfig = std::move(j);
	tv::Logger::Log(tv::LogLevel::Notice, tv::InfoType::Application,
					fmt::format("Load : {}", config_file.generic_string()));
}
Config::Config(const json& j) : _jconfig(new json(j)) {}
Config::Config(const Config& config) = default;
Config::Config(const Config& config, std::string_view str)
	: _jconfig(config._jconfig), _base(std::string(str)) {}
Config::Config(Config&& config) noexcept : _base(config._base.to_string()) {
	if (this != &config) {
		std::swap(_jconfig, config._jconfig);
	}
}
Config::~Config() = default;
// Config& Config::operator=(const Config& config) {
// 	_jconfig = config._jconfig;
// 	return *this;
// }
Config& Config::operator=(Config&& config) noexcept {
	if (this != &config) {
		_jconfig = std::move(config._jconfig);
	}
	return *this;
}
const json& Config::Json() const { return *_jconfig; }
Config      Config::Relative(std::string_view key) const {
    return Config(*this, key);
}
const json& Config::Schema(std::string_view key) const {
	return _get_schema(_base, (key[0] == '/' ? "" : "/") + std::string(key));
}
const nlohmann::json& Config::_key_value(const nlohmann::json& schema,
										 std::string_view      key) const {
	std::string path(key);
	if (Config::_argument().contains(path)) {
		return Config::_argument()[path];
	}
	if (_jconfig->contains(path)) {
		return _jconfig->operator[](path);
	}
	return schema.at("default");
}

Config Config::Get(std::string_view key) { return _config.Relative(key); }
void   Config::CommandLineOptions(const nlohmann::json& j) {
    if (_command_line_argument.empty()) {
        _command_line_argument = j;
    }
}
const Config Config::_config                = _initialize("setting.json");
json         Config::_command_line_argument = json();
const json   Config::_jschema               = Config::_load_schema();
Config       Config::_initialize(const std::filesystem::path& file) noexcept {
    try {
        return Config(file);
    }
    catch (std::exception&) {
        return Config(json::object());
    }
}
const json& Config::_argument() { return _command_line_argument; }
json        Config::_load_schema() {
#include "../doc/schemas/setting_schema.json.gen.h"
	return json::parse(setting_json);
}
const json& Config::_get_schema(const _json_pointer& p, std::string_view key) {
	auto path = [](std::string_view str) -> std::string {
		std::istringstream iss((std::string(str)));
		std::ostringstream oss;
		for (std::string obj; std::getline(iss, obj, '/');) {
			oss << "/properties" << obj;
		}
		return oss.str();
	};
	return Config::_jschema.at(path(p.to_string()) + path(key));
}
bool Config::_check_schema_value(const nlohmann::json& schema,
								 const nlohmann::json& value) {
	auto                             def = schema.find("default");
	const std::array<std::string, 6> types{"boolean", "integer", "number",
										   "string",  "array",   "object"};
	//schemaとvalueが同じタイプ
	auto result = std::find(std::cbegin(types), std::cend(types),
							schema.at("type").get<std::string>());
	return result != std::cend(types) && *result == value.type_name();
}
} // namespace glapp

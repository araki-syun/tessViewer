#include "config.h"
#include "fmt/core.h"
#include <nlohmann/json.hpp>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <array>

#include "../log.h"

using tv::InfoType;
using tv::Logger;
using tv::LogLevel;
namespace glapp {
using namespace nlohmann;
Config::Config(const std::filesystem::path& config_file) {
	std::ifstream ifs(config_file);
	ifs.exceptions(std::ios_base::failbit);
	auto j = std::shared_ptr<json>();
	ifs >> *j;
	_jconfig = std::move(j);
	Logger::Log<LogLevel::Notice>(
		InfoType::Application,
		fmt::format("Config Load : {}", config_file.generic_string()));
}
Config::Config(const json& j) : _jconfig(new json(j)) {}
Config::Config(const Config& config) = default;
Config::Config(const Config& config, std::string_view str)
	: _jconfig(config._jconfig)
	, _base(config._base / Config::_jptr_from_str(str)) {
	Logger::Log<LogLevel::Trace>(
		InfoType::Application,
		fmt::format("Config Create Relative : {}", _base.to_string()));
}
Config::Config(Config&& config) noexcept : _base(config._base) {
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
	try {
		return _get_schema(_base, Config::_jptr_from_str(key));
	}
	catch (detail::out_of_range& e) {
		throw tv::AppError(
			tv::LogLevel::Error,
			fmt::format(
				"スキーマが存在しません。\n{}\njson error code : {}\n{}",
				(_base / Config::_jptr_from_str(key)).to_string(), e.id,
				e.what()));
	}
}
const json& Config::_key_value(const json&         schema,
							   const json_pointer& key) const {
	auto path = _base / key;
	if (Config::_argument().contains(path)) {
		Logger::Log<LogLevel::Trace>( InfoType::Application, "Config Value From Program Options");
		return Config::_argument()[path];
	}
	if (_jconfig->contains(path)) {
		Logger::Log<LogLevel::Trace>( InfoType::Application, "Config Value From setting.json");
		return _jconfig->operator[](path);
	}
	Logger::Log<LogLevel::Trace>( InfoType::Application, "Config Value From Schema Default");
	return schema.at("default");
}

Config Config::Get(std::string_view key) { return _config.Relative(key); }
void   Config::CommandLineOptions(const json& j) {
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
const json& Config::_get_schema(const json_pointer& p,
								const json_pointer& key) {
	auto path = [](json_pointer ptr) -> json_pointer {
		json_pointer j;
		json_pointer prop("/properties");
		while (!ptr.empty()) {
			j = prop / ptr.back() / j;
			ptr.pop_back();
		}
		return j;
	};
	auto key_path = json_pointer(std::string(key));
	return Config::_jschema.at(path(p) / path(key_path));
}
bool Config::_check_schema_value(const json& schema, const json& value) {
	auto                             def = schema.find("default");
	const std::array<std::string, 7> types{
		"null", "boolean", "number", "string", "array", "object", "discarded"};
	//schemaとvalueが同じタイプ
	auto result = std::find(std::cbegin(types), std::cend(types),
							schema.at("type").get<std::string>());
	return result != std::cend(types) ? *result == value.type_name() : true;
}
Config::json_pointer Config::_jptr_from_str(std::string_view str) {
	if (str.empty()) {
		return json_pointer();
	}
	return json_pointer((!str.empty() && str[0] == '/' ? "" : "/") +
						std::string(str));
}
} // namespace glapp

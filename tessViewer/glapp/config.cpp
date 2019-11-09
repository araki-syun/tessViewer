#include "config.h"
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

namespace glapp {
using namespace nlohmann;
Config::Config(const std::filesystem::path& config_file) {
	try {
		std::ifstream ifs(config_file);
		ifs.exceptions(std::ios_base::failbit);
		auto j = std::shared_ptr<json>();
		ifs >> *j;
		_jconfig = std::move(j);
	}
	catch (std::exception& e) {
		std::cerr << e.what();
	}
}
Config::Config(const Config& config) = default;
Config::Config(const Config& config, std::string_view str)
	: _jconfig(config._jconfig), _base(std::string(str)) {}
Config::Config(Config&& config) noexcept : _base(config._base.to_string()) {
	if (this != &config) {
		std::swap(_jconfig, config._jconfig);
	}
}
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
const json& Config::Json() const { return *_jconfig; }
Config Config::Relative(std::string_view key) { return Config(*this, key); }
const nlohmann::json& Config::_key_value(const nlohmann::json& schema,
										 std::string_view      str) const {
	using namespace std::string_literals;
	const auto& j   = *_jconfig;
	auto        def = schema.find("default");
	auto        it  = j.find(std::string(str));

	const std::array<std::string, 5> types{"boolean", "integer", "number",
										   "string", "null"};
	//スキーマが値型
	if (auto result = std::find(std::cbegin(types), std::cend(types),
								schema.at("type").get<std::string>());
		result != std::cend(types)) {
		//キーが存在する
		if (it != j.end() && it->type_name() == *result) {
			return *it;
		}
		//スキーマのデフォルト値が存在する
		if (def != schema.end()) {
			return *def;
		}
		throw std::domain_error("key : " + std::string(str) + "\n" +
								"keyのデフォルト値は存在しません");
	}
	throw std::domain_error("key : " + std::string(str) + "\n" +
							"keyは値型ではありません");
}
const Config Config::_config = Config("setting.json");
;
const json Config::_jschema = Config::_load_schema();
json       Config::_load_schema() {
#include "../doc/schemas/setting_schema.json.gen.h"
	return json::parse(setting_json);
}
const json& Config::_get_schema(std::string_view key) {
	std::istringstream iss((std::string(key)));
	std::ostringstream oss;
	for (std::string obj; std::getline(iss, obj, '/');) {
		oss << "/properties" << obj;
	}
	return Config::_jschema.at(oss.str());
}
} // namespace glapp

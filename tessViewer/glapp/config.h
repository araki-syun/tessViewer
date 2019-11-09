#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <string_view>
#include <type_traits>

#include "nlohmann/detail/json_pointer.hpp"
#include "nlohmann/json_fwd.hpp"
#include "nlohmann/json.hpp"
#include "glm_conversion.h"

namespace glapp {
template <class, class = void>
struct is_less_then_comparable : std::false_type {};

template <class T>
struct is_less_then_comparable<
	T,
	std::void_t<decltype(std::declval<T&>() < std::declval<T&>())>>
	: std::true_type {};

class Config {
public:
	Config() = delete;
	Config(const std::filesystem::path& config_file);
	Config(const Config& config);
	Config(const Config& config, std::string_view str);
	Config(Config&& config) noexcept;
	~Config();
	Config& operator=(const Config& config);
	Config& operator=(Config&& config) noexcept;

	const nlohmann::json& Json() const;
	template <class T>
	T Value(std::string_view key) const {
		auto schema = Config::_get_schema(key);
		return _round_value(schema, _key_value(schema, key).get<T>());
	}
	Config Relative(std::string_view key);

private:
	std::shared_ptr<nlohmann::json>              _jconfig;
	const nlohmann::json_pointer<nlohmann::json> _base;

	const nlohmann::json& _key_value(const nlohmann::json& schema,
									 std::string_view      str) const;

public:
	static const Config& GetSingleton() { return _config; }

private:
	static const Config          _config;
	static const nlohmann::json  _jschema;
	static nlohmann::json        _load_schema();
	static const nlohmann::json& _get_schema(std::string_view key);
	template <class T>
	static T _round_value(const nlohmann::json& schema, const T& value) {
		if constexpr (is_less_then_comparable<T>::value) {
			auto maximum = schema.find("maximum");
			auto minimum = schema.find("minimum");
			auto end     = schema.cend();
			if (maximum != end && minimum != end) {
				return std::clamp(value, maximum->get<T>(), minimum->get<T>());
			}
			auto v = value;
			if (maximum != schema.cend()) {
				v = std::min(value, maximum->get<T>());
			}
			if (minimum != schema.cend()) {
				v = std::max(value, minimum->get<T>());
			}
			return v;
		}
		return value;
	}
};
} // namespace glapp

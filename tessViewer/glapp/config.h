#pragma once

#include <stdexcept>
#include <string>
#include <filesystem>
#include <optional>
#include <algorithm>

#include "nlohmann/json_fwd.hpp"
#include "nlohmann/json.hpp"
#include <string_view>

namespace glapp {
class Config {
public:
	Config() = delete;
	Config(const std::filesystem::path& config_file);
	Config(const Config& config);
	Config(Config&& config) noexcept;
	~Config();
	Config& operator=(const Config& config);
	Config& operator=(Config&& config) noexcept;

	const nlohmann::json& Json();
	template <class T, class... ARGS>
	T Value(ARGS... args) const {
		return _key_value(args...).get<T>();
	}

private:
	nlohmann::json _jconfig;

	template <class... ARGS>
	nlohmann::json& _key_value(const ARGS&... keys) {
		using namespace std::string_literals;
		auto& j   = _jconfig;
		auto& sch = Config::_jschema;
		// try {
		for (auto& key : {keys...}) { // schemaからkeyをたどる
			if (auto it_prop = sch.find("properties"); it_prop != sch.cend()) {
				if (auto it_pkey = it_prop->find(key);
					it_pkey != it_prop->cend()) {
					// 次のschとなるkeyをpropertiesから参照する
					sch = *it_pkey;
				} else {
					throw std::domain_error("key : "s + key + "\n"s +
											"keyは存在しません");
				}
			}
		}
		for (auto& key : {keys...}) { // json設定ファイルからkeyをたどる
			if (auto it = j.find(key);
				it != j.cend()) { // it が有効なら j に入れる
				j = *it;
			} else { // json設定ファイルにkeysは存在しない
				j = nlohmann::json::object(); // 無効値
				break;
			}
		}
		constexpr std::array<std::string> types{"boolean", "integer", "number",
												"string", "null"};
		// schが値型かどうか
		if (auto result = std::find(std::cbegin(types), std::cend(types),
									sch.at("type").get<std::string>());
			result != std::cend(types)) {
			if (j.empty()) { // 設定ファイルにkeyが存在しない場合
				if (auto def = sch.find("default"); def != it_prop->cend()) {
					return *def;
				} else {
					throw std::domain_error("key : "s + key + "\n"s +
											"keyのデフォルト値は存在しません");
				}
			}
			// 設定ファイルにkeyが存在し、jとschが同じ型かどうか
			if (j.type_name() == *result) {
				//   jとschが同じ型の場合
				// スキーマの最大値と最小値で丸める
				auto value = j.get<T>();
				if (auto maximum = sch.find("maximum"); max != sch.cend()) {
					value = std::min(value, maximum->get<T>());
				}
				if (auto minimum = sch.find("minimum"); min != sch.cend()) {
					value = std::max(value, minimum->get<T>());
				}
				j = value;
				return j;
			} else {
				//   jとschが違う型の場合
				throw std::domain_error("keyのタイプが違います。\n"
										"correct key type  : " +
										*result + "\n"s +
										"argument key type : " + j.type_name() +
										"\n"s);
			}
		}
		// schが配列型の場合
		else if (sch.at("type").get<std::string>() == "array") {
			if (j.empty()) {
				if (auto& def = sch.find("default"); def != it_prop->cend()) {
					return *def;
				} else {
					throw std::domain_error("key : "s + key + "\n"s +
											"keyのデフォルト値は存在しません");
				}
			}
			// jが配列型か確認
			if (j.type() == nlohmann::detail::value_t::array) {
				//配列の長さを確認
				auto maxItems = sch.find("maxItems");
				if (maxItems != sch.cend()) {
					//規定より長かったら切り詰める
					if (auto max = maxItems->get<int>(); j.size() > max) {
						nlohmann::json tmp;
						for (int i = 0; i < max; ++i) {
							tmp.emplace_back(j.at(i));
						}
						j = tmp;
					}
				}
				if (auto minItems = sch.find("minItems");
					minItems != sch.cend()) {
					auto min = minItems->get<int>();
					if (min > max) {
						throw std::domain_error("key : "s + key + "\n"s +
												"スキーマが不正です\n")
					}
					//規定より短かったら足りない部分はデフォルト値で埋める
					if (j.size() < min) {
						if (auto def = sch.find("default"); def != sch.cend()) {
							for (auto i = j.size(); i < min; ++i) {
								j.emplace_back(def->at(i));
							}
						} else {
							throw std::domain_error(
								"key : "s + key + "\n"s +
								"keyのデフォルト値は存在しません");
						}
					}
				}
				return j;
			}
		}

		using namespace std::string_literals;
		throw std::domain_error("key : "s + key + "\n"s +
								"keyは見つかりませんでした");
	}

public:
	static const Config& GetSingleton() { return _config; }

private:
	static const Config         _config;
	static const nlohmann::json _jschema;
};
} // namespace glapp

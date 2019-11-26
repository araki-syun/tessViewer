#pragma once

#include <ostream>
#include <fstream>
#include <string>
#include <string_view>
#include <memory>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <nlohmann/detail/macro_scope.hpp>

#include "exception.h"

namespace tv {
enum class LogLevel { None = 0, Fatal, Error, Warning, Notice, Debug, Trace };
enum class InfoType { Unknown, Application, Graphics, UserInterface, Control };
std::string ToString(LogLevel lv);
std::string ToString(InfoType type);
enum class OutputType { Quiet, File, Stdout };

/**
 * @brief 指定した出力先にログメッセージを出力する
 * 
 */
class Logger {
public:
	/**
	 * @brief 初期化
	 * 
	 * @param lv ログレベル
	 * これより上のレベルのメッセージは出力されない
	 * 
	 * @param output メッセージの出力先
	 * std::ostream派生のオブジェクトのストリームバッファを指定する
	 * 
	 * @example Initialize(Loglevel::Debug, std::cout.rdbuf());
	 */
	static void Initialize(LogLevel lv, std::streambuf* output);

	/**
	 * @brief コンストラクタで指定した出力に対してログを出力する
	 * 
	 * @param lv メッセージのレベル
	 * 
	 * @param type メッセージの種類
	 * 
	 * @param str メッセージ
	 */
	static void Log(LogLevel lv, InfoType type, std::string_view str);
	static void Log(const AppError& e);

private:
	Logger(LogLevel lv, std::streambuf* output);
	void           _log(LogLevel lv, InfoType type, std::string_view str);
	const LogLevel _level = LogLevel::None;
	std::ostream   _output;

	inline static std::unique_ptr<Logger> _logger;
};
} // namespace tv
NLOHMANN_JSON_SERIALIZE_ENUM(
	tv::OutputType, {{Quiet, "quiet"}, {File, "file"}, {Stdout, "stdout"}});

// LogLevelをフォーマット出来るようにする
namespace fmt {
template <>
struct formatter<tv::LogLevel> {
	template <typename PARSE_CONTEXT>
	constexpr auto parse(PARSE_CONTEXT& ctx) { //NOLINT
		return ctx.begin();
	}

	template <typename FORMAT_CONTEXT>
	auto format(const tv::LogLevel& lv, FORMAT_CONTEXT& ctx) { //NOLINT
		return format_to(ctx.out(), "{s}", tv::ToString(lv));
	}
};
// InfoTypeをフォーマット出来るようにする
template <>
struct formatter<tv::InfoType> {
	template <typename PARSE_CONTEXT>
	constexpr auto parse(PARSE_CONTEXT& ctx) { //NOLINT
		return ctx.begin();
	}

	template <typename FORMAT_CONTEXT>
	auto format(const tv::InfoType& type, FORMAT_CONTEXT& ctx) { //NOLINT
		return format_to(ctx.out(), "{s}", tv::ToString(type));
	}
};
} // namespace fmt
namespace nlohmann {
template <>
struct adl_serializer<tv::LogLevel> { //NOLINT
	template <typename BASIC_JSON_TYPE>
	static void to_json(BASIC_JSON_TYPE& j, const tv::LogLevel& v) { //NOLINT
		j = static_cast<int>(v);
	}
	template <typename BASIC_JSON_TYPE>
	static void from_json(const BASIC_JSON_TYPE& j, tv::LogLevel& v) { //NOLINT
		v = static_cast<tv::LogLevel>(j.get<int>());
	}
};
template <>
struct adl_serializer<tv::InfoType> { //NOLINT
	template <typename BASIC_JSON_TYPE>
	static void to_json(BASIC_JSON_TYPE& j, const tv::InfoType& v) { //NOLINT
		j = static_cast<int>(v);
	}
	template <typename BASIC_JSON_TYPE>
	static void from_json(const BASIC_JSON_TYPE& j, tv::InfoType& v) { //NOLINT
		v = static_cast<tv::InfoType>(j.get<int>());
	}
};
} // namespace nlohmann

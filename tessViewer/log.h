#pragma once

#include <ostream>
#include <fstream>
#include <string>
#include <string_view>
#include <memory>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "exception.h"

#ifndef COMPILE_TIME_LOG_LEVEL
#define COMPILE_TIME_LOG_LEVEL Notice
#endif

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
	template <LogLevel LV = LogLevel::Trace>
	static void Log(InfoType type, std::string_view str) {
		if constexpr (LV <= _compile_time_level) {
			if (_logger) {
				_logger->_log(LV, type, str);
			}
		}
	}
	static void Log(const AppError& e);

	static constexpr LogLevel CompileTimeLogLevel() {
		return _compile_time_level;
	}

private:
	Logger(LogLevel lv, std::streambuf* output);
	void           _log(LogLevel lv, InfoType type, std::string_view str);
	const LogLevel _level = LogLevel::None;
	std::ostream   _output;

	inline static std::unique_ptr<Logger> _logger;
	static constexpr LogLevel             _compile_time_level =
		LogLevel::COMPILE_TIME_LOG_LEVEL;
};
} // namespace tv

template <class T,
		  std::enable_if_t<std::is_enum_v<T>, std::nullptr_t> = nullptr>
constexpr bool operator<(const T& l, const T& r) {
	if constexpr (!std::is_enum_v<T>) {
		static_assert("T != enum");
	}
	using type = std::underlying_type_t<T>;
	return static_cast<type>(l) < static_cast<type>(r);
}

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
	//NOLINTNEXTLINE
	static void to_json(BASIC_JSON_TYPE& j, const tv::LogLevel& v) {
		j = static_cast<int>(v);
	}
	template <typename BASIC_JSON_TYPE>
	//NOLINTNEXTLINE
	static void from_json(const BASIC_JSON_TYPE& j, tv::LogLevel& v) {
		v = static_cast<tv::LogLevel>(j.get<int>());
	}
};
template <>
struct adl_serializer<tv::InfoType> { //NOLINT
	template <typename BASIC_JSON_TYPE>
	//NOLINTNEXTLINE
	static void to_json(BASIC_JSON_TYPE& j, const tv::InfoType& v) {
		j = static_cast<int>(v);
	}
	template <typename BASIC_JSON_TYPE>
	//NOLINTNEXTLINE
	static void from_json(const BASIC_JSON_TYPE& j, tv::InfoType& v) {
		v = static_cast<tv::InfoType>(j.get<int>());
	}
};
template <>
struct adl_serializer<tv::OutputType> { //NOLINT
	template <typename BASIC_JSON_TYPE>
	//NOLINTNEXTLINE
	static void to_json(BASIC_JSON_TYPE& j, const tv ::OutputType& e) {
		static_assert(std ::is_enum<tv ::OutputType>::value,
					  "tv::OutputType"
					  " must be an enum!");
		static const std ::pair<tv ::OutputType, BASIC_JSON_TYPE> m[] = {
			{tv ::OutputType ::Quiet, "quiet"},
			{tv ::OutputType ::File, "file"},
			{tv ::OutputType ::Stdout, "stdout"}};
		auto it = std ::find_if(
			std ::begin(m), std ::end(m),
			[e](const std ::pair<tv ::OutputType, BASIC_JSON_TYPE>& ej_pair)
				-> bool { return ej_pair.first == e; });
		j = ((it != std ::end(m)) ? it : std ::begin(m))->second;
	}
	template <typename BASIC_JSON_TYPE>
	//NOLINTNEXTLINE
	static void from_json(const BASIC_JSON_TYPE& j, tv ::OutputType& e) {
		static_assert(std ::is_enum<tv ::OutputType>::value,
					  "tv::OutputType"
					  " must be an enum!");
		static const std ::pair<tv ::OutputType, BASIC_JSON_TYPE> m[] = {
			{tv ::OutputType ::Quiet, "quiet"},
			{tv ::OutputType ::File, "file"},
			{tv ::OutputType ::Stdout, "stdout"}};
		auto it = std ::find_if(
			std ::begin(m), std ::end(m),
			[j](const std ::pair<tv ::OutputType, BASIC_JSON_TYPE>& ej_pair)
				-> bool { return ej_pair.second == j; });
		e = ((it != std ::end(m)) ? it : std ::begin(m))->first;
	}
};
} // namespace nlohmann

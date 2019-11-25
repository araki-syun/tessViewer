#include "log.h"
#include <fmt/core.h>
#include <fmt/chrono.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>
#include <typeinfo>
#include <utility>

namespace tv {
std::string ToString(LogLevel lv) {
	switch (lv) {
	case LogLevel::None: return "None";
	case LogLevel::Fatal: return "Fatal";
	case LogLevel::Error: return "Error";
	case LogLevel::Warning: return "Warning";
	case LogLevel::Notice: return "Notice";
	case LogLevel::Debug: return "Debug";
	case LogLevel::Trace: return "Trace";
	}
}
std::string ToString(InfoType type) {
	switch (type) {
	case InfoType::Unknown: return "Unknown";
	case InfoType::Application: return "Application";
	case InfoType::Graphics: return "Graphics";
	case InfoType::UserInterface: return "UserInterface";
	case InfoType::Control: return "Control";
	}
}

void Logger::Initialize(LogLevel lv, std::streambuf* output) {
	if (!_logger) {
		_logger.reset(new Logger(lv, output)); //NOLINT
	}
}
void Logger::Log(LogLevel lv, InfoType type, std::string_view str) {
	if (_logger) {
		_logger->_log(lv, type, str);
	}
}
Logger::Logger(LogLevel lv, std::streambuf* output)
#ifdef _DEBUG //リリースでは詳細なログを取得できないように
	: _level(lv)
#else
	: _level(std::min(lv, LogLevel::Notice))
#endif
	, _output(std::forward<std::streambuf*>(output)) {
	if (auto out = dynamic_cast<std::filebuf*>(_output.rdbuf());
		out != nullptr && !out->is_open()) {
		out->open("tessViewer.log", std::ios_base::out | std::ios_base::app);
	}
}
void Logger::_log(LogLevel lv, InfoType type, std::string_view str) {
	if (_level < lv || lv == LogLevel::None) {
		return;
	}
	auto    now = std::time(nullptr);
	std::tm tm{};
	localtime_s(&tm, &now);
	_output << fmt::format("[ {:s} ] {:%F %T} {:s}\n",
						   ToString(type), //NOLINTNEXTLINE
						   tm, std::string(str))
			<< std::endl;
}
} // namespace tv

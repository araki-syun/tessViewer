#include "log.h"

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
		_logger = std::make_unique<Logger>(lv, output);
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
} // namespace tv

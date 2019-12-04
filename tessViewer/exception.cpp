#include "exception.h"
#include <string_view>

#include "log.h"

namespace tv {
AppError::AppError(LogLevel lv, std::string_view message)
	: std::runtime_error(std::string(message)), _lv(lv) {}
LogLevel AppError::Level() const { return _lv; }
InfoType AppError::Info() const { return InfoType::Application; }

InfoType GraphicsError::Info() const { return InfoType::Graphics; }
InfoType InterfaceError::Info() const { return InfoType::UserInterface; }
InfoType ControlError::Info() const { return InfoType::Control; }
} // namespace tv

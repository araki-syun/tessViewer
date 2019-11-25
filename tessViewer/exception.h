#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace tv {
enum class LogLevel;
enum class InfoType;

class AppError : public std::runtime_error {
public:
	AppError(LogLevel lv, std::string_view message);

	LogLevel         Level() const;
	virtual InfoType Info() const;

private:
	LogLevel _lv;
};
class GraphicsError : public AppError {
public:
	using AppError::AppError;
	InfoType Info() const override;
};
class InterfaceError : public AppError {
public:
	using AppError::AppError;
	InfoType Info() const override;
};
class ControlError : public AppError {
public:
	using AppError::AppError;
	InfoType Info() const override;
};

} // namespace tv

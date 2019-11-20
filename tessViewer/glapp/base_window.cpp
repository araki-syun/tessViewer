#include "base_window.h"

namespace glapp::inner {
//BaseWindow::BaseWindow(void)
//{
//}
BaseWindow::BaseWindow(std::string_view title) : _title(title) {}
BaseWindow::~BaseWindow() = default;

std::string_view BaseWindow::GetTitle() const { return _title; }
} // namespace glapp::inner

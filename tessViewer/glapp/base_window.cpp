#include "base_window.h"

namespace glapp::inner {
//base_window::base_window(void)
//{
//}
base_window::base_window(std::string_view title) : _title(title) {}
base_window::~base_window() = default;

std::string_view base_window::GetTitle() const { return _title; }
} // namespace glapp::inner

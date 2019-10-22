#pragma once

#include <cstdint>

namespace glapp {
namespace inner {
class base_window {
public:
	enum init_flag : unsigned int {
		DEFAULT    = 0x00000000,
		FULLSCREEN = 0x00000001
	};
	base_window(void) = delete;
	explicit base_window(const char* title,
						 int         width  = 1280,
						 int         height = 720,
						 init_flag   flag   = init_flag::DEFAULT);
	virtual ~base_window(void) = 0;
	base_window(base_window&)  = delete; // コピーコンストラクタ
	base_window& operator=(const base_window&) = delete; // コピー代入演算子
	base_window(base_window&&) = delete; // ムーブコンストラクタ
	base_window& operator=(const base_window&&) = delete; // ムーブ代入演算子

	virtual void GetWindowSize(int* w, int* h) const     = 0;
	virtual void GetWindowPosition(int* x, int* y) const = 0;
};
} // namespace inner
} // namespace glapp

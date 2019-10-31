#pragma once

#include <cstdint>

namespace glapp::inner {
class base_window {
public:
	enum class init_flag : unsigned int {
		DEFAULT    = 0x00000000,
		FULLSCREEN = 0x00000001
	};
	base_window() = delete;
	explicit base_window(const char* title,
						 int         width  = 1280,
						 int         height = 720,
						 init_flag   flag   = init_flag::DEFAULT);
	virtual ~base_window() = 0;
	base_window(base_window&) = delete; // �R�s�[�R���X�g���N�^
	base_window& operator=(const base_window&) = delete; // �R�s�[������Z�q
	base_window(base_window&&) = delete; // ���[�u�R���X�g���N�^
	base_window& operator=(const base_window&&) = delete; // ���[�u������Z�q

	virtual void GetWindowSize(int* w, int* h) const     = 0;
	virtual void GetWindowPosition(int* x, int* y) const = 0;
};
} // namespace glapp::inner

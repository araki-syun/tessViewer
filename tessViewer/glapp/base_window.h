#pragma once

#include <cstdint>
#include <string_view>

namespace glapp::inner {
class base_window {
public:
	base_window() = delete;
	explicit base_window(std::string_view title);
	virtual ~base_window() = 0;
	base_window(base_window&) = delete; // �R�s�[�R���X�g���N�^
	base_window& operator=(const base_window&) = delete; // �R�s�[������Z�q
	base_window(base_window&&) = delete; // ���[�u�R���X�g���N�^
	base_window& operator=(const base_window&&) = delete; // ���[�u������Z�q

	virtual void     GetWindowSize(int* w, int* h) const     = 0;
	virtual void     GetWindowPosition(int* x, int* y) const = 0;
	std::string_view GetTitle() const;

protected:
	std::string _title;
};
} // namespace glapp::inner

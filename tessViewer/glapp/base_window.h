#pragma once

#include <cstdint>
#include <string_view>

namespace glapp::inner {
class BaseWindow {
public:
	BaseWindow() = delete;
	explicit BaseWindow(std::string_view title);
	virtual ~BaseWindow() = 0;
	BaseWindow(BaseWindow&) = delete; // �R�s�[�R���X�g���N�^
	BaseWindow& operator=(const BaseWindow&) = delete; // �R�s�[������Z�q
	BaseWindow(BaseWindow&&) = delete; // ���[�u�R���X�g���N�^
	BaseWindow& operator=(const BaseWindow&&) = delete; // ���[�u������Z�q

	virtual void     GetWindowSize(int* w, int* h) const     = 0;
	virtual void     GetWindowPosition(int* x, int* y) const = 0;
	std::string_view GetTitle() const;

protected:
	std::string _title;
};
} // namespace glapp::inner

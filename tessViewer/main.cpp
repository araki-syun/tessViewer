#include "app.h"

#include <iostream>
#include <boost\program_options.hpp>

#include "location_define.h"
#include "version.h"

#include "glapp\config.h"
#include "glapp\glapp_define.h"

void main(int argc, char* argv[]) {
	using namespace boost::program_options;
	options_description options("option");
	options_description window_option("display_option");
	options_description graphics_option("graphics_option");
	options_description ui_option("ui_option");

	options.add_options()
		("help,h", "�w���v��\��")
		("version,v", "�o�[�W�������")
		("object,o", value<std::string>()->default_value(MODEL "cube_uv.sdmj"), "�\�����f��");

	window_option.add_options()
		(GLAPP_CONFIG_FULLSCREEN ",f", "�S��ʕ\��")
		(GLAPP_CONFIG_RESOLUTION_X, value<int>()->default_value(1280), "���𑜓x")
		(GLAPP_CONFIG_RESOLUTION_Y, value<int>()->default_value(720), "�c�𑜓x")
		(GLAPP_CONFIG_FOV, value<float>()->default_value(60.f), "Fov")
		(GLAPP_CONFIG_VSYNC, value<bool>()->default_value(true), "VSync");

	graphics_option.add_options()
		(GLAPP_CONFIG_PATCH_TYPE_GREGORY ",g", "Patch Type �� GREGORY_BASIS ���g�p")
		(GLAPP_CONFIG_PATCH_LEVEL_DEFAULT ",p", value<int>()->default_value(2), "�f�t�H���g�p�b�`���x��")
		(GLAPP_CONFIG_PATCH_LEVEL_MAX, value<int>()->default_value(6), "�ő�p�b�`���x��")
		(GLAPP_CONFIG_TESS_LEVEL_DEFAULT ",t", value<int>()->default_value(1), "�f�t�H���g�e�b�Z���[�V�����W��")
		(GLAPP_CONFIG_TESS_LEVEL_MAX, value<int>()->default_value(6), "�ő�e�b�Z���[�V�����W��");

	ui_option.add_options()
		(GLAPP_CONFIG_USER_INTERFACE, value<bool>()->default_value(true), "���[�U�[�C���^�[�t�F�C�X")
		(GLAPP_CONFIG_FONT_FILE, value<std::string>()->default_value(FONT "ipaexg.ttf"), "�g�p����t�H���g")
		(GLAPP_CONFIG_FONT_SIZE, value<int>()->default_value(16), "�t�H���g�T�C�Y")
		(GLAPP_CONFIG_FONT_COLOR, value<std::uint8_t>()->default_value(0), "�t�H���g�F");

	options.add(window_option).add(graphics_option).add(ui_option);

	boost::program_options::variables_map vm;

	try {
		store(parse_command_line(argc, argv, options), vm);
		//store(parse_command_line(argc, argv, window_option), vm);
		//store(parse_command_line(argc, argv, graphics_option), vm);
		//store(parse_command_line(argc, argv, ui_option), vm);
		notify(vm);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return;
	}
	if (vm.count("version")) {
		std::cout << "tessViewer " TV_VERSION << std::endl;
		return;
	}
	if (vm.count("help")) {
		std::cout << "tessViewer " TV_VERSION << std::endl;
		std::cout << options << std::endl;
		//std::cout << window_option << std::endl;
		//std::cout << graphics_option << std::endl;
		//std::cout << ui_option << std::endl;
		return;
	}

	try {
		app a(vm);
		a.Run();
	}
	catch (boost::bad_any_cast& e) {
		std::cerr << e.what() << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "error log output : tessviewer.log" << std::endl;
		std::ofstream ost(".//tessviewer.log", std::ios::trunc);
		ost << e.what() << std::endl;

	}
	catch (const char* message) {
		std::cerr << message << std::endl;
	}
	catch (...) {
		std::cerr << "Unknown Exception\n" << std::endl;
	}
}


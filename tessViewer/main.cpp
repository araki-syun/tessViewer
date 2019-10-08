#include "app.h"

#include <iostream>
#include <string>
#include <boost\program_options.hpp>

#include "location_define.h"
#include "version.h"

#include "glapp\config.h"
#include "glapp\glapp_define.h"

void main(int argc, char* argv[]) {
	using namespace std::literals::string_literals;
	using namespace boost::program_options;
	options_description options("option");
	options_description window_option("display_option");
	options_description graphics_option("graphics_option");
	options_description ui_option("ui_option");

	options.add_options()
		("help,h", "ヘルプを表示")
		("version,v", "バージョン情報")
		("object,o", value<std::string>()->default_value(MODEL "cube_uv.sdmj"), "表示モデル");

	window_option.add_options()
		(GLAPP_CONFIG_FULLSCREEN ",f", "全画面表示")
		(GLAPP_CONFIG_RESOLUTION_X, value<int>()->default_value(1280), "横解像度")
		(GLAPP_CONFIG_RESOLUTION_Y, value<int>()->default_value(720), "縦解像度")
		(GLAPP_CONFIG_FOV, value<float>()->default_value(60.f), "Fov")
		(GLAPP_CONFIG_VSYNC, value<bool>()->default_value(true), "VSync");

	graphics_option.add_options()
		(GLAPP_CONFIG_PATCH_TYPE_GREGORY ",g", "Patch Type に GREGORY_BASIS を使用")
		(GLAPP_CONFIG_PATCH_LEVEL_DEFAULT ",p", value<int>()->default_value(2), "デフォルトパッチレベル")
		(GLAPP_CONFIG_PATCH_LEVEL_MAX, value<int>()->default_value(6), "最大パッチレベル")
		(GLAPP_CONFIG_TESS_LEVEL_DEFAULT ",t", value<int>()->default_value(1), "デフォルトテッセレーション係数")
		(GLAPP_CONFIG_TESS_LEVEL_MAX, value<int>()->default_value(6), "最大テッセレーション係数");

	ui_option.add_options()
		(GLAPP_CONFIG_USER_INTERFACE, value<bool>()->default_value(true), "ユーザーインターフェイス")
		//(GLAPP_CONFIG_FONT_FILE, value<std::string>()->default_value(FONT R"(ipaexg.ttf)"), R"(使用するフォント)")
		(GLAPP_CONFIG_FONT_SIZE, value<int>()->default_value(16), "フォントサイズ")
		(GLAPP_CONFIG_FONT_COLOR, value<std::uint8_t>()->default_value(0), "フォント色");

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


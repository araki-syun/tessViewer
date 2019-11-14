#include "app.h"

#include <iostream>
#include <string>
#include <fstream>

#include <boost\program_options.hpp>
#include <string_view>

#include "boost/program_options/variables_map.hpp"
#include "location_define.h"
#include "nlohmann/json_fwd.hpp"
#include "version.h"

#include "glapp\config.h"
#include "glapp\glapp_define.h"

void import_options(const boost::program_options::variables_map& vm);

int main(int argc, char* argv[]) {
	using namespace std::literals::string_literals;
	using namespace boost::program_options;
	options_description options("option");
	options_description window_option("display_option");
	options_description graphics_option("graphics_option");
	options_description ui_option("ui_option");

	options.add_options()("help,h", "ヘルプを表示")(
		"version,v", "バージョン情報")("object,o", value<std::string>(),
									   "表示モデル");

	window_option.add_options()(GLAPP_CONFIG_FULLSCREEN ",f", value<bool>(),
								"全画面表示")(GLAPP_CONFIG_RESOLUTION_X,
											  value<int>(), "横解像度")(
		GLAPP_CONFIG_RESOLUTION_Y, value<int>(),
		"縦解像度")(GLAPP_CONFIG_FOV, value<float>(),
					"Fov")(GLAPP_CONFIG_VSYNC, value<bool>(), "VSync");

	graphics_option.add_options()(GLAPP_CONFIG_PATCH_TYPE_GREGORY ",g",
								  "Patch Type に GREGORY_BASIS を使用")(
		GLAPP_CONFIG_PATCH_LEVEL_DEFAULT ",p", value<int>(),
		"デフォルトパッチレベル")(GLAPP_CONFIG_PATCH_LEVEL_MAX, value<int>(),
								  "最大パッチレベル")(
		GLAPP_CONFIG_TESS_LEVEL_DEFAULT ",t", value<int>(),
		"デフォルトテッセレーション係数")(
		GLAPP_CONFIG_TESS_LEVEL_MAX, value<int>(), "最大テッセレーション係数");

	ui_option.add_options()(GLAPP_CONFIG_USER_INTERFACE, value<bool>(),
							"ユーザーインターフェイス")
		//(GLAPP_CONFIG_FONT_FILE, value<std::string>()->default_value(FONT R"(ipaexg.ttf)"), R"(使用するフォント)")
		(GLAPP_CONFIG_FONT_SIZE, value<int>(), "フォントサイズ")(
			GLAPP_CONFIG_FONT_COLOR, value<std::uint8_t>(), "フォント色");

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
		return -1;
	}
	if (vm.count("version") != 0u) {
		std::cout << "tessViewer " TV_VERSION << std::endl;
		return 0;
	}
	if (vm.count("help") != 0u) {
		std::cout << "tessViewer " TV_VERSION << std::endl;
		std::cout << options << std::endl;
		//std::cout << window_option << std::endl;
		//std::cout << graphics_option << std::endl;
		//std::cout << ui_option << std::endl;
		return 0;
	}

	try {
		app a;
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

void import_options(const boost::program_options::variables_map& vm) {
	using nlohmann::json;
	json options;
	json window;
	json graphics;
	json ui;
	auto contain = [&](std::string_view name) -> bool {
		return vm.count(std::string(name)) > 0;
	};

	if (contain(GLAPP_CONFIG_FULLSCREEN)) {
		window["fullscreen"] = vm[GLAPP_CONFIG_FULLSCREEN].as<bool>();
	}
	if (contain(GLAPP_CONFIG_RESOLUTION_X)) {
		window["/resolution/width"] = vm[GLAPP_CONFIG_RESOLUTION_X].as<int>();
	}
	if (contain(GLAPP_CONFIG_RESOLUTION_Y)) {
		window["/resolution/height"] = vm[GLAPP_CONFIG_RESOLUTION_Y].as<int>();
	}
	if (contain(GLAPP_CONFIG_FOV)) {
		window["fov"] = vm[GLAPP_CONFIG_FOV].as<float>();
	}
	if (contain(GLAPP_CONFIG_VSYNC)) {
		window["vsync"] = vm[GLAPP_CONFIG_VSYNC].as<bool>();
	}
	if (contain("object")) {
		graphics["/graphics/model"] = vm["object"].as<std::string>();
	}
	if (contain(GLAPP_CONFIG_PATCH_LEVEL_DEFAULT)) {
		graphics["/graphics/osd/patch/level"] =
			vm[GLAPP_CONFIG_PATCH_LEVEL_DEFAULT].as<int>();
	}
	if (contain(GLAPP_CONFIG_TESS_LEVEL_DEFAULT)) {
		graphics["/graphics/osd/tessellation/level"] =
			vm[GLAPP_CONFIG_TESS_LEVEL_DEFAULT].as<int>();
	}
	if (contain(GLAPP_CONFIG_FONT_SIZE)) {
		ui["/ui/font/size"] = vm[GLAPP_CONFIG_FONT_SIZE].as<int>();
	}
	if (contain(GLAPP_CONFIG_FONT_COLOR)) {
		ui["/ui/font/color"] = vm[GLAPP_CONFIG_FONT_COLOR].as<std::string>();
	}
	options["window"]   = window;
	options["graphics"] = graphics;
	options["ui"]       = ui;

	glapp::Config::CommandLineOptions(options);
}

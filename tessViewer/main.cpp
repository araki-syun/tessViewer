#include "app.h"

#include <exception>
#include <iostream>
#include <string>
#include <fstream>

#include <boost\program_options.hpp>
#include <string_view>
#include <fmt/format.h>

#include "boost/program_options/value_semantic.hpp"
#include "boost/program_options/variables_map.hpp"
#include <nlohmann/json.hpp>
#include "version.h"

#include "glapp\config.h"
#include "log.h"
#include "define.h"

using namespace std::literals::string_literals;
using namespace boost::program_options;
using namespace fmt::literals;
using namespace tv;

void ImportOptions(const variables_map& vm);

int main(int argc, char* argv[]) {
	auto format = "{},{}"_format;

	options_description options("option");
	options_description window_option("display_option");
	options_description graphics_option("graphics_option");
	options_description ui_option("ui_option");

	// clang-format off
	options.add_options()
		(format("help", 'h').c_str(), "ヘルプを表示")
		(format("version", "v").c_str(), "バージョン情報")
		(format("object", "o").c_str(), value<std::string>(), "表示モデル")
		("log_output", value<std::string>()->default_value("quiet"), "ログの出力先")
		("log_level", value<int>()->default_value(0), "ログの詳細度");

	window_option.add_options()
		(format(GLAPP_CONFIG_FULLSCREEN, "f").c_str(), value<bool>(),
		 "全画面表示")
		(GLAPP_CONFIG_RESOLUTION_X, value<int>(), "横解像度")
		(GLAPP_CONFIG_RESOLUTION_Y, value<int>(), "縦解像度")
		(GLAPP_CONFIG_FOV, value<float>(),
				   "Fov")(GLAPP_CONFIG_VSYNC, value<bool>(), "VSync");

	graphics_option.add_options()
		(format(GLAPP_CONFIG_PATCH_TYPE_GREGORY, "g").c_str(),
		 "Patch Type に GREGORY_BASIS を使用")
		(format(GLAPP_CONFIG_PATCH_LEVEL_DEFAULT, "p").c_str(),
		 value<int>(), "デフォルトパッチレベル")
		(GLAPP_CONFIG_PATCH_LEVEL_MAX, value<int>(),
		 "最大パッチレベル")
		(format(GLAPP_CONFIG_TESS_LEVEL_DEFAULT, "t").c_str(),
		 value<int>(), "デフォルトテッセレーション係数")
		(GLAPP_CONFIG_TESS_LEVEL_MAX, value<int>(),
				   "最大テッセレーション係数");

	ui_option.add_options()
		(GLAPP_CONFIG_USER_INTERFACE, value<bool>(),
		 "ユーザーインターフェイス")
		(GLAPP_CONFIG_FONT_SIZE, value<int>(), "フォントサイズ")
		(GLAPP_CONFIG_FONT_COLOR, value<std::uint8_t>(),
				   "フォント色");
	// clang-format on

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
		std::cout << "tessViewer {}"_format(TV_VERSION) << std::endl;
		return 0;
	}
	if (vm.count("help") != 0u) {
		std::cout << "tessViewer {}\n"_format(TV_VERSION) << options
				  << std::endl;
		return 0;
	}

	auto ofs = std::ofstream();
	try {
		ImportOptions(vm);

		auto conf_log = glapp::Config::Get("/application/log");
		auto lv       = conf_log.Value<tv::LogLevel>("level");
		switch (conf_log.Value<tv::OutputType>("output")) {
		case tv::OutputType::Quiet: break;
		case tv::OutputType::File: Logger::Initialize(lv, ofs.rdbuf()); break;
		case tv::OutputType::Stdout:
			Logger::Initialize(lv, std::cout.rdbuf());
			break;
		}

		App a;
		a.Run();
		return 0;
	}
	catch (boost::bad_any_cast& e) {
		Logger::Log<LogLevel::Fatal>(InfoType::Application, e.what());
	}
	catch (tv::AppError& e) {
		Logger::Log(e);
	}
	catch (std::exception& e) {
		Logger::Log<LogLevel::Fatal>(InfoType::Application, e.what());
	}
	catch (const char* message) {
		Logger::Log<LogLevel::Fatal>(InfoType::Application, message);
	}
	catch (...) {
		Logger::Log<LogLevel::Fatal>(InfoType::Unknown, "Unknown Error");
	}
	return 1;
}

void ImportOptions(const boost::program_options::variables_map& vm) {
	using nlohmann::json;
	json app;
	json options;
	json window;
	json graphics;
	json ui;
	auto contain = [&](std::string_view name) -> bool {
		return vm.count(std::string(name)) > 0;
	};

	if (contain("log_output")) {
		app["/log/output"_json_pointer] = vm["log_output"].as<std::string>();
	}
	if (contain("log_level")) {
		app["/log/level"_json_pointer] = vm["log_level"].as<int>();
	}
	if (contain(GLAPP_CONFIG_FULLSCREEN)) {
		window["/fullscreen"_json_pointer] =
			vm[GLAPP_CONFIG_FULLSCREEN].as<bool>();
	}
	if (contain(GLAPP_CONFIG_RESOLUTION_X)) {
		window["/resolution/width"_json_pointer] =
			vm[GLAPP_CONFIG_RESOLUTION_X].as<int>();
	}
	if (contain(GLAPP_CONFIG_RESOLUTION_Y)) {
		window["/resolution/height"_json_pointer] =
			vm[GLAPP_CONFIG_RESOLUTION_Y].as<int>();
	}
	if (contain(GLAPP_CONFIG_FOV)) {
		window["fov"] = vm[GLAPP_CONFIG_FOV].as<float>();
	}
	if (contain(GLAPP_CONFIG_VSYNC)) {
		window["vsync"] = vm[GLAPP_CONFIG_VSYNC].as<bool>();
	}
	if (contain("object")) {
		graphics["/graphics/model"_json_pointer] =
			vm["object"].as<std::string>();
	}
	if (contain(GLAPP_CONFIG_PATCH_LEVEL_DEFAULT)) {
		graphics["/graphics/osd/patch/level"_json_pointer] =
			vm[GLAPP_CONFIG_PATCH_LEVEL_DEFAULT].as<int>();
	}
	if (contain(GLAPP_CONFIG_TESS_LEVEL_DEFAULT)) {
		graphics["/graphics/osd/tessellation/level"_json_pointer] =
			vm[GLAPP_CONFIG_TESS_LEVEL_DEFAULT].as<int>();
	}
	if (contain(GLAPP_CONFIG_FONT_SIZE)) {
		ui["/ui/font/size"_json_pointer] = vm[GLAPP_CONFIG_FONT_SIZE].as<int>();
	}
	if (contain(GLAPP_CONFIG_FONT_COLOR)) {
		ui["/ui/font/color"_json_pointer] =
			vm[GLAPP_CONFIG_FONT_COLOR].as<std::string>();
	}
	options["application"] = app;
	options["window"]      = window;
	options["graphics"]    = graphics;
	options["ui"]          = ui;

	glapp::Config::CommandLineOptions(options);
}

#pragma once

#include <string>

#define ___DATA R"(.\data)"

namespace tv {
constexpr char DATA[]    = ___DATA;
constexpr char FONT[]    = ___DATA R"(\font\)";
constexpr char SHADER[]  = ___DATA R"(\shader\)";
constexpr char MODEL[]   = ___DATA R"(\model\)";
constexpr char TEXTURE[] = ___DATA R"(\texture\)";

constexpr char GLAPP_OSD_COMMON_SOURCE_STRING[] = "osd_common_shader";
constexpr char GLAPP_OSD_VERTEX_SOURCE_STRING[] = "osd_vertex_shader";
constexpr char GLAPP_OSD_TESS_CONTROL_SOURCE_STRING[] =
	"osd_tess_control_shader";
constexpr char GLAPP_OSD_TESS_EVAL_SOURCE_STRING[] = "osd_tess_eval_shader";

constexpr char GLAPP_CONFIG_WINDOW[]   = "Window.";
constexpr char GLAPP_CONFIG_GRAPHICS[] = "Graphics.";
constexpr char GLAPP_CONFIG_UI[]       = "UI.";

constexpr char GLAPP_CONFIG_FULLSCREEN[]          = "full_screen";
constexpr char GLAPP_CONFIG_RESOLUTION_X[]        = "resolution_x";
constexpr char GLAPP_CONFIG_RESOLUTION_Y[]        = "resolution_y";
constexpr char GLAPP_CONFIG_FOV[]                 = "fov";
constexpr char GLAPP_CONFIG_VSYNC[]               = "vsync";
constexpr char GLAPP_CONFIG_PATCH_TYPE_GREGORY[]  = "osd_patch_type_gregory";
constexpr char GLAPP_CONFIG_PATCH_LEVEL_DEFAULT[] = "osd_patch_level_default";
constexpr char GLAPP_CONFIG_PATCH_LEVEL_MAX[]     = "osd_patch_level_max";
constexpr char GLAPP_CONFIG_TESS_LEVEL_DEFAULT[]  = "osd_tess_level_default";
constexpr char GLAPP_CONFIG_TESS_LEVEL_MAX[]      = "osd_tess_level_max";
constexpr char GLAPP_CONFIG_USER_INTERFACE[]      = "user_interface";
constexpr char GLAPP_CONFIG_FONT_FILE[]           = "font_file";
constexpr char GLAPP_CONFIG_FONT_SIZE[]           = "font_size";
constexpr char GLAPP_CONFIG_FONT_COLOR[]          = "font_color";

inline const std::string FONT_VERTEX_SHADER =
	std::string(SHADER) + R"(default_font.vert)";
inline const std::string FONT_FRAGMENT_SHADER =
	std::string(SHADER) + R"(default_font.frag)";

inline const std::string DEFAULT_VERTEX_SHADER =
	std::string(SHADER) + "default.vert";
inline const std::string DEFAULT_FRAGMENT_SHADER =
	std::string(SHADER) + "default.frag";
inline const std::string DEFAULT_GEOMETRY_SHADER =
	std::string(SHADER) + "default.geom";
inline const std::string DEFAULT_TESS_CTRL_SHADER =
	std::string(SHADER) + "default.tcs";
inline const std::string DEFAULT_TESS_EVAL_SHADER =
	std::string(SHADER) + "default.tes";
inline const std::string COMMON_SHADER = std::string(SHADER) + "common.glsl";

} // namespace tv

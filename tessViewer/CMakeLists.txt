cmake_minimum_required(VERSION 3.16.0)

message("CMAKE_TOOLCHAIN_FILE : ${CMAKE_TOOLCHAIN_FILE}")
message("VCPKG_TARGET_TRIPLET : ${VCPKG_TARGET_TRIPLET}")
message("COMPILE_TIME_LOG_LEVEL : ${COMPILE_TIME_LOG_LEVEL}")

find_package(Python3 3.4 REQUIRED)
message("Python3 executable ${Python3_EXECUTABLE}")

find_path(GLM_INCLUDE_DIRS				"glm/glm.hpp")
find_path(nlohmann_json_INCLUDE_DIRS	"nlohmann/json.hpp")
find_package(OpenGL		REQUIRED)
find_package(GLEW		REQUIRED)
find_package(GLFW3		REQUIRED)
find_package(fmt		REQUIRED)
find_package(freetype	REQUIRED)
find_package(OpenCV		REQUIRED)
find_package(jpeg		REQUIRED)
find_package(liblzma	REQUIRED)
find_package(libpng		REQUIRED)
find_package(webp		REQUIRED)
find_package(bzip2		REQUIRED)
find_package(tiff		REQUIRED)
find_package(zlib		REQUIRED)
find_path(Ptex_INCLUDE_DIRS "Ptexture.h")
find_library(Ptex_LIBRARY "Ptex.lib")
find_package(Boost 1.71.0 COMPONENTS program_options REQUIRED)
set(OpenSubdiv_INCLUDE_DIRS "" CACHE PATH "OpenSubdiv include directory.")
set(OpenSubdiv_LIBRARY_DIRS "" CACHE PATH "OpenSubdiv library directory.")
set(OpenSubdiv_CPU_LIB "${OpenSubdiv_LIBRARY_DIRS}/osdCPU.lib")
set(OpenSubdiv_GPU_LIB "${OpenSubdiv_LIBRARY_DIRS}/osdGPU.lib")

add_executable(${PROJECT_NAME})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

# setting_schema.json.gen.h 生成
add_custom_command(
	OUTPUT doc/schemas/setting_schema.json.gen.h
	COMMAND ${Python3_EXECUTABLE} doc/schemas/cpp_header_gen.py
	MAIN_DEPENDENCY doc/schemas/setting_schema.json
	DEPENDS doc/schemas/setting_schema.json
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "generated ${CMAKE_CURRENT_SOURCE_DIR}/doc/schemas/setting_schema.json.gen.h"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(TESSVIEWER_LOG_LEVELS None Fatal Error Warning Notice Debug Trace)
set(COMPILE_TIME_LOG_LEVEL Notice CACHE STRING "compile time Log Level")
set_property(CACHE COMPILE_TIME_LOG_LEVEL PROPERTY STRINGS ${TESSVIEWER_LOG_LEVELS})
if(NOT COMPILE_TIME_LOG_LEVEL IN_LIST TESSVIEWER_LOG_LEVELS)
	message(FATAL_ERROR "COMPILE_TIME_LOG_LEVEL(${COMPILE_TIME_LOG_LEVEL}) is not included in the TESSVIEWER_LOG_LEVELS")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
	GLEW_STATIC
	OSD_USES_GLEW
	NOMINMAX
	_USE_MATH_DEFINES
	BOOST_ALL_NO_LIB
	COMPILE_TIME_LOG_LEVEL=${COMPILE_TIME_LOG_LEVEL}
)
target_include_directories(${PROJECT_NAME} PRIVATE
	${OpenGL_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIRS}
	${GLFW3_INCLUDE_DIRS}
	${GLM_INCLUDE_DIRS}
	${nlohmann_json_INCLUDE_DIRS}
	${fmt_INCLUDE_DIRS}
	${FREETYPE_INCLUDE_DIR_freetype2}
	${FREETYPE_INCLUDE_DIR_ft2build}
	${OpenCV_INCLUDE_DIRS}
	${Protobuf_INCLUDE_DIR}
	${JPEG_INCLUDE_DIR}
	${LIBLZMA_INCLUDE_DIR}
	${PNG_PNG_INCLUDE_DIR}
	${WEBP_INCLUDE_DIR}
	${BZIP2_INCLUDE_DIR}
	${TIFF_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${ptex_INCLUDE_DIRS}
	${OpenSubdiv_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)
target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.h")
target_sources(${PROJECT_NAME} PRIVATE
	version.h
	doc/schemas/setting_schema.json.gen.h
	main.cpp
	app.cpp
	camera.cpp
	exception.cpp
	glQuery.cpp
	glShaderVariable.cpp
	glslProgram.cpp
	glslStringDraw.cpp
	log.cpp
	main.cpp
	material.cpp
	model.cpp
	osd_info.cpp
#	pch.cpp
	Setting.cpp
	ShaderManager.cpp
	glapp/base_window.cpp
	glapp/config.cpp
	glapp/glapp.cpp
	glapp/osd_program.cpp
	glapp/shader_manager.cpp
	glapp/shader_object.cpp
	glapp/shader_program.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
	${OPENGL_gl_LIBRARY}
	GLEW::GLEW
	glfw
	fmt::fmt
	${FREETYPE_LIBRARY}
	${OpenCV_LIBS}
	${Protobuf_LIBRARY}
	${JPEG_LIBRARY}
	${LIBLZMA_LIBRARIES}
	${PNG_LIBRARY}
	${WEBP_BASE_LIBRARY}
	${BZIP2_LIBRARY}
	${TIFF_LIBRARY}
	${ZLIB_LIBRARY}
	${Ptex_LIBRARY}
	${OpenSubdiv_CPU_LIB}
	${OpenSubdiv_GPU_LIB}
	Boost::program_options
)

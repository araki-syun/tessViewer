#pragma once

#include <algorithm>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <utility>
#include <stdexcept>
#include <filesystem>

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <boost/program_options.hpp>

#include <opensubdiv\osd\opengl.h>
#include <opensubdiv\osd\glMesh.h>
#include <opensubdiv\osd\glComputeEvaluator.h>
#include <opensubdiv\osd\glVertexBuffer.h>
#include <opensubdiv\far\topologyDescriptor.h>
#include <opensubdiv\far\topologyRefiner.h>
#include <opensubdiv\far\stencilTable.h>
#include <opensubdiv\far\primvarRefiner.h>
#include <opensubdiv\far\error.h>

#ifdef _WINDOWS_
#undef near
#undef far
#endif

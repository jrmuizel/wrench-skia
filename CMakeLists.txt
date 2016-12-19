# Boilerplate.
cmake_minimum_required (VERSION 3.1)   # First version with CMAKE_CXX_STANDARD.
project (skimake)
set (CMAKE_CXX_STANDARD 11)

if (MSVC)
  set(VIEWER_MANIFEST viewer.exe.manifest)
endif()

add_subdirectory(yaml-cpp)
include_directories(yaml-cpp/include)
include_directories(yaml-cpp/include/yaml-cpp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

add_executable(viewer viewer.cpp yaml.cpp Color.cpp FloatRoundedRect.cpp FloatPoint.cpp BoxBorderPainter.cpp BorderEdge.cpp ObjectPainter.cpp ${VIEWER_MANIFEST})

target_link_libraries(viewer skia yaml-cpp ${OPENGL_LIBRARIES} ${WIN32_LIBS} glfw)

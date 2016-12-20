# Boilerplate.
cmake_minimum_required (VERSION 3.1)   # First version with CMAKE_CXX_STANDARD.
project (skimake)
set (CMAKE_CXX_STANDARD 11)
include(ExternalProject)

set(SKIA_ARGS "--args=is_official_build=true skia_use_piex=false skia_enable_tools=false extra_cflags=[\"-w\"]")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
   set(SKIA_ARGS, "${SKIA_ARGS} is_debug=true")
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

ExternalProject_Add(
        skia
        DOWNLOAD_COMMAND gclient config --unmanaged https://skia.googlesource.com/skia.git && gclient sync --revision d1c550e71fa72cfbf03b93c10ec8b9d1d109f8c2
        # gn has this dumb bug where it will chop off the SOURCE_DIR string from the front of the path
        # if it is a string prefix instead of a path prefx so we need make sure that BINARY_DIR doesn't
        # SOURCE_DIR as a prefix
        BINARY_DIR out
        CONFIGURE_COMMAND cd <SOURCE_DIR> && python  bin/fetch-gn && gn gen <BINARY_DIR> ${SKIA_ARGS}
        BUILD_COMMAND ninja
        INSTALL_COMMAND  true
)

ExternalProject_Get_Property(skia source_dir)
set(SKIA_INCLUDE_DIR ${source_dir}/include)
ExternalProject_Get_Property(skia binary_dir)
set(SKIA_BIN_DIR ${binary_dir})

find_package(OpenGL REQUIRED)
list (APPEND libs  ${OPENGL_LIBRARIES})
if (UNIX AND NOT APPLE)
	find_package (Freetype)
	find_package (GIF)



	find_path (FONTCONFIG_INCLUDE_DIRS "fontconfig/fontconfig.h")
	find_library (FONTCONFIG_LIBRARIES fontconfig)

	if (FREETYPE_FOUND)
	    list (APPEND private_includes ${FREETYPE_INCLUDE_DIRS})
	    list (APPEND libs             ${FREETYPE_LIBRARIES})
	endif()

	if (FONTCONFIG_INCLUDE_DIRS AND FONTCONFIG_LIBRARIES)
	    list (APPEND private_includes ${FONTCONFIG_INCLUDE_DIRS})
	    list (APPEND libs             ${FONTCONFIG_LIBRARIES})
	endif()

	# We require libjpeg-turbo >= 1.5.0.
	find_package(PkgConfig)
	pkg_check_modules(JPEG_TURBO libjpeg>=1.5.0)
endif()


if (MSVC)
  set(VIEWER_MANIFEST viewer.exe.manifest)
endif()

set(skia_INCLUDE_DIRS skia/include)

add_subdirectory(yaml-cpp)
include_directories(yaml-cpp/include)
include_directories(yaml-cpp/include/yaml-cpp)
include_directories(${SKIA_INCLUDE_DIR})
include_directories(${SKIA_INCLUDE_DIR}/config)
include_directories(${SKIA_INCLUDE_DIR}/core)
include_directories(${SKIA_INCLUDE_DIR}/gpu)
include_directories(${SKIA_INCLUDE_DIR}/utils)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

add_executable(viewer viewer.cpp yaml.cpp Color.cpp FloatRoundedRect.cpp FloatPoint.cpp BoxBorderPainter.cpp BorderEdge.cpp ObjectPainter.cpp ${VIEWER_MANIFEST})

add_dependencies(viewer skia)
target_link_libraries(viewer yaml-cpp ${SKIA_BIN_DIR}/libskia.a ${libs} ${WIN32_LIBS} glfw)

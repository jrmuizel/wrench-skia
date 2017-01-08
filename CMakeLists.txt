# Boilerplate.
cmake_minimum_required (VERSION 3.1)   # First version with CMAKE_CXX_STANDARD.
project (skimake)
set (CMAKE_CXX_STANDARD 11)
include(ExternalProject)

set(SKIA_ARGS "is_official_build=true skia_use_piex=false skia_enable_tools=false")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
   set(SKIA_ARGS ${SKIA_ARGS} is_debug=true)
endif()

if(MSVC)
  set(CMD_C cmd /c)
  set(SKIA_LIB_NAME skia.lib)
  set(SKIA_BUILD_COMMAND cmd /s /c "cd <SOURCE_DIR> && python bin/fetch-gn && gn gen <BINARY_DIR> --args=\"${SKIA_ARGS}\"")

  # build using the static runtime, to match skia
  foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
  endforeach(flag_var)
else()
  set(CMD_C "")
  set(SKIA_LIB_NAME libskia.a)
  set(SKIA_ARGS "${SKIA_ARGS} extra_cflags=[\"-w\"]")
  set(SKIA_BUILD_COMMAND cd <SOURCE_DIR> && python bin/fetch-gn && gn gen <BINARY_DIR> --args=${SKIA_ARGS})
endif()

ExternalProject_Add(
        skia
        DOWNLOAD_COMMAND ${CMD_C} gclient config --unmanaged https://skia.googlesource.com/skia.git && ${CMD_C} gclient sync --revision d1c550e71fa72cfbf03b93c10ec8b9d1d109f8c2
        # gn has this dumb bug where it will chop off the SOURCE_DIR string from the front of the path
        # if it is a string prefix instead of a path prefx so we need make sure that BINARY_DIR doesn't
        # SOURCE_DIR as a prefix
        BINARY_DIR out
        CONFIGURE_COMMAND ${SKIA_BUILD_COMMAND}
        BUILD_COMMAND ninja
        INSTALL_COMMAND echo
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

add_subdirectory(yaml-cpp)
include_directories(yaml-cpp/include)
include_directories(yaml-cpp/include/yaml-cpp)
include_directories(${SKIA_INCLUDE_DIR})
include_directories(${SKIA_INCLUDE_DIR}/config)
include_directories(${SKIA_INCLUDE_DIR}/core)
include_directories(${SKIA_INCLUDE_DIR}/gpu)
include_directories(${SKIA_INCLUDE_DIR}/utils)
link_directories(${SKIA_BIN_DIR})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

add_executable(viewer viewer.cpp yaml.cpp Color.cpp FloatRoundedRect.cpp FloatPoint.cpp BoxBorderPainter.cpp BorderEdge.cpp ObjectPainter.cpp ${VIEWER_MANIFEST})

add_dependencies(viewer skia)
target_link_libraries(viewer yaml-cpp ${SKIA_LIB_NAME} ${libs} ${WIN32_LIBS} glfw)

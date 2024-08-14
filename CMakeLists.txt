cmake_minimum_required(VERSION 3.14)

project(personal-site-server)

include(FetchContent)

FetchContent_Declare(
	cpp-types
	GIT_REPOSITORY https://github.com/TomBeacock/cpp-types.git
	GIT_TAG origin/main
)

FetchContent_Declare(
	cpp-web
	GIT_REPOSITORY https://github.com/TomBeacock/cpp-web.git
	GIT_TAG origin/main
)

FetchContent_Declare(
	cpp-json
	GIT_REPOSITORY https://github.com/TomBeacock/cpp-json.git
	GIT_TAG origin/main
)

FetchContent_MakeAvailable(cpp-types cpp-web cpp-json)

add_executable(personal-site-server 
	"src/main.cpp"	"src/endian.h"	"src/read_write_mutex.h" "src/read_write_mutex.cpp"	"src/res/record.h"	"src/res/database.h" "src/res/database.cpp"	"src/web/web_server.h" "src/web/web_server.cpp"	"src/content/content_server.h" "src/content/content_server.cpp")

set_target_properties(personal-site-server PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_include_directories(personal-site-server
	PRIVATE
		"${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(personal-site-server PUBLIC cpp-types cpp-web cpp-json)

string(LENGTH "${PROJECT_SOURCE_DIR}/" source_path_length)
target_compile_definitions(personal-site-server PUBLIC SOURCE_PATH_LENGTH=${source_path_length})

# Build type
if(CMAKE_BUILD_TYPE)
	string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
endif()

if(build_type STREQUAL "debug")
	target_compile_definitions(personal-site-server PUBLIC BUILD_DEBUG PUBLIC LOGGING_ENABLED)
elseif(build_type STREQUAL "relwithdebinfo")
	target_compile_definitions(personal-site-server PUBLIC BUILD_RELEASE_WITH_INFO PUBLIC LOGGING_ENABLED)
elseif(build_type STREQUAL "release")
	target_compile_definitions(personal-site-server PUBLIC BUILD_RELEASE)
endif()

# Platform
if(UNIX)
	target_compile_definitions(personal-site-server PUBLIC PLATFORM_UNIX)
endif()

if(WIN32)
	target_compile_definitions(personal-site-server PUBLIC PLATFORM_WIN)
endif()

# Compiler
if(MSVC)
	target_compile_options(personal-site-server PUBLIC /Zc:preprocessor)
endif()
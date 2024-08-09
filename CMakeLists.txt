cmake_minimum_required(VERSION 3.14)

project(web-server)

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

FetchContent_MakeAvailable(cpp-types cpp-web)

add_executable(web-server 
	"src/main.cpp"	"src/web/web_server.h" "src/web/web_server.cpp")

set_target_properties(web-server PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_include_directories(web-server
	PRIVATE
		"${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(web-server PUBLIC cpp-types cpp-web)

string(LENGTH "${PROJECT_SOURCE_DIR}/" source_path_length)
target_compile_definitions(web-server PUBLIC SOURCE_PATH_LENGTH=${source_path_length})

# Build type
if(CMAKE_BUILD_TYPE)
	string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
endif()

if(build_type STREQUAL "debug")
	target_compile_definitions(web-server PUBLIC BUILD_DEBUG PUBLIC LOGGING_ENABLED)
elseif(build_type STREQUAL "relwithdebinfo")
	target_compile_definitions(web-server PUBLIC BUILD_RELEASE_WITH_INFO PUBLIC LOGGING_ENABLED)
elseif(build_type STREQUAL "release")
	target_compile_definitions(web-server PUBLIC BUILD_RELEASE)
endif()

# Platform
if(UNIX)
	target_compile_definitions(web-server PUBLIC PLATFORM_UNIX)
endif()

if(WIN32)
	target_compile_definitions(web-server PUBLIC PLATFORM_WIN)
endif()

# Compiler
if(MSVC)
	target_compile_options(web-server PUBLIC /Zc:preprocessor)
endif()
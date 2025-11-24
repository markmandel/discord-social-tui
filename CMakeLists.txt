# Copyright 2025 Mark Mandel
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.30)
project(discord_social_tui
        VERSION 0.1.0
        DESCRIPTION "Terminal UI for Discord social features"
        LANGUAGES CXX)

# C++ settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# RPATH settings for finding shared libraries
if (UNIX AND NOT APPLE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
elseif (APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif ()

# Dependencies
include(FetchContent)

# FTXUI for terminal UI
FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v6.1.9
)

# spdlog for logging
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.16.0
)

# Make dependencies available
FetchContent_MakeAvailable(ftxui spdlog)

# Discord Social SDK
set(DISCORD_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/discord_social_sdk)
set(DISCORD_SDK_INCLUDE_DIR ${DISCORD_SDK_DIR}/include)
set(DISCORD_SDK_LIB_DIR ${DISCORD_SDK_DIR}/lib/release)

# Get all source files in src/app directory
file(GLOB APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/app/*.cpp")

# Library
add_library(${PROJECT_NAME}_lib ${APP_SOURCES})

target_include_directories(${PROJECT_NAME}_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
        ${DISCORD_SDK_INCLUDE_DIR}
)

# Get Discord SDK library path based on platform
if(WIN32)
    set(DISCORD_LIB_PATH "${DISCORD_SDK_DIR}/lib/release/discord_partner_sdk.lib")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_DIR}/bin/release/discord_partner_sdk.dll")
elseif(APPLE)
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.dylib")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.dylib")
else() # Linux
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.so")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.so")
endif()

target_link_libraries(${PROJECT_NAME}_lib PUBLIC
        ftxui::screen
        ftxui::dom
        ftxui::component
        ${DISCORD_LIB_PATH}
        spdlog::spdlog
)

# Executable
add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}_lib
)

# Install
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Copy dynamic libraries to build directory for all platforms
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DISCORD_SHARED_LIB}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Also install the dynamic libraries
install(FILES
        ${DISCORD_SHARED_LIB}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Custom target for doing formatting and linting

# Find all source files
file(GLOB_RECURSE SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/includes/*.hpp
        ${CMAKE_SOURCE_DIR}/includes/*.h)

# Add lint target for running clang-format and clang-tidy
find_program(CLANG_FORMAT clang-format)
find_program(CLANG_TIDY clang-tidy)
find_program(RUN_CLANG_TIDY run-clang-tidy)

set(TIDY_CHECKS "bugprone-*,cppcoreguidelines-*,modernize-*,performance-*,portability-*,readability-*,-modernize-use-trailing-return-type,-readability-magic-numbers,-bugprone-lambda-function-name")

# Determine number of parallel jobs (use nproc if available, otherwise default)
if(UNIX)
    execute_process(COMMAND nproc OUTPUT_VARIABLE NUM_CORES
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT NUM_CORES)
        set(NUM_CORES 4)
    endif()
else()
    set(NUM_CORES 4)
endif()

if (CLANG_FORMAT AND RUN_CLANG_TIDY)
    add_custom_target(lint
            COMMAND ${CMAKE_COMMAND} -E echo "Running clang-format on source files..."
            COMMAND ${CLANG_FORMAT} -i ${SOURCE_FILES}
            COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy on source files (parallel with ${NUM_CORES} jobs)..."
            COMMAND ${RUN_CLANG_TIDY} -j ${NUM_CORES} -p ${CMAKE_BINARY_DIR} -checks=${TIDY_CHECKS} -quiet ${SOURCE_FILES}
            COMMENT "Running code quality tools"
            VERBATIM
    )
elseif (CLANG_FORMAT AND CLANG_TIDY)
    add_custom_target(lint
            COMMAND ${CMAKE_COMMAND} -E echo "Running clang-format on source files..."
            COMMAND ${CLANG_FORMAT} -i ${SOURCE_FILES}
            COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy on source files..."
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CLANG_TIDY} -checks=${TIDY_CHECKS} ${SOURCE_FILES} -- -std=c++23 -I${CMAKE_SOURCE_DIR}/build/_deps/ftxui-src/include -I${CMAKE_SOURCE_DIR}/build/_deps/spdlog-src/include -I${CMAKE_SOURCE_DIR}/includes -I${DISCORD_SDK_INCLUDE_DIR}
            COMMENT "Running code quality tools"
            VERBATIM
    )
elseif (CLANG_FORMAT)
    add_custom_target(lint
            COMMAND ${CLANG_FORMAT} -i ${SOURCE_FILES}
            COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found. Only formatting was performed."
            COMMENT "Running clang-format only"
            VERBATIM
    )
elseif (RUN_CLANG_TIDY)
    add_custom_target(lint
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Only linting was performed."
            COMMAND ${RUN_CLANG_TIDY} -j ${NUM_CORES} -p ${CMAKE_BINARY_DIR} -checks=${TIDY_CHECKS} -quiet ${SOURCE_FILES}
            COMMENT "Running clang-tidy only (parallel)"
            VERBATIM
    )
elseif (CLANG_TIDY)
    add_custom_target(lint
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Only linting was performed."
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CLANG_TIDY} -checks=${TIDY_CHECKS} ${SOURCE_FILES} -- -std=c++23 -I${CMAKE_SOURCE_DIR}/build/_deps/ftxui-src/include -I${CMAKE_SOURCE_DIR}/build/_deps/spdlog-src/include -I${CMAKE_SOURCE_DIR}/includes -I${DISCORD_SDK_INCLUDE_DIR}
            COMMENT "Running clang-tidy only"
            VERBATIM
    )
else ()
    add_custom_target(lint
            COMMAND ${CMAKE_COMMAND} -E echo "Neither clang-format nor clang-tidy found. Please install these tools to enable the lint target."
            COMMENT "Code quality tools not found"
            VERBATIM
    )
endif ()
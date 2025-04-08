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

# Dependencies
include(FetchContent)

FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v6.0.2
)

FetchContent_MakeAvailable(ftxui)

# Library 
add_library(${PROJECT_NAME}_lib 
        src/app/app.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

target_link_libraries(${PROJECT_NAME}_lib PUBLIC
        ftxui::screen
        ftxui::dom
        ftxui::component
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

# Custom target for doing formatting and linting

# Find all source files
file(GLOB_RECURSE SOURCE_FILES 
     ${CMAKE_SOURCE_DIR}/src/*.cpp
     ${CMAKE_SOURCE_DIR}/includes/*.hpp
     ${CMAKE_SOURCE_DIR}/includes/*.h)

# Add lint target for running clang-format and clang-tidy
find_program(CLANG_FORMAT clang-format)
find_program(CLANG_TIDY clang-tidy)

set(TIDY_CHECKS "-checks=bugprone-*,cppcoreguidelines-*,modernize-*,performance-*,portability-*,readability-*,-modernize-use-trailing-return-type,-readability-magic-numbers")

if(CLANG_FORMAT AND CLANG_TIDY)
    add_custom_target(lint
        COMMAND ${CMAKE_COMMAND} -E echo "Running clang-format on source files..."
        COMMAND ${CLANG_FORMAT} -i ${SOURCE_FILES}
        COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy on source files..."
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CLANG_TIDY} ${TIDY_CHECKS} ${SOURCE_FILES} -- -std=c++23 -I${CMAKE_SOURCE_DIR}/build/_deps/ftxui-src/include -I${CMAKE_SOURCE_DIR}/includes
        COMMENT "Running code quality tools"
        VERBATIM
    )
elseif(CLANG_FORMAT)
    add_custom_target(lint
        COMMAND ${CLANG_FORMAT} -i ${SOURCE_FILES}
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found. Only formatting was performed."
        COMMENT "Running clang-format only"
        VERBATIM
    )
elseif(CLANG_TIDY)
    add_custom_target(lint
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Only linting was performed."
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CLANG_TIDY} ${TIDY_CHECKS} ${SOURCE_FILES} -- -std=c++23 -I${CMAKE_SOURCE_DIR}/build/_deps/ftxui-src/include -I${CMAKE_SOURCE_DIR}/includes
        COMMENT "Running clang-tidy only"
        VERBATIM
    )
else()
    add_custom_target(lint
        COMMAND ${CMAKE_COMMAND} -E echo "Neither clang-format nor clang-tidy found. Please install these tools to enable the lint target."
        COMMENT "Code quality tools not found"
        VERBATIM
    )
endif()
cmake_minimum_required(VERSION 3.18)
project(projet_pasteque C)

set(CMAKE_C_STANDARD 11)
# When creating a new C file, make sure to add it to this list!
set(SOURCE_FILES
        src/main.c)

add_executable(projet_pasteque ${SOURCE_FILES})
target_include_directories(projet_pasteque PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include/)

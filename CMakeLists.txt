cmake_minimum_required(VERSION 3.7)
project(projet_pasteque C)
set(CMAKE_C_STANDARD 11)

option(PASTEQUE_ENABLE_PROFILING "Enables profiling and debugging data" OFF)

# Common functions
function(add_common_compile_options target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /D_CRT_SECURE_NO_WARNINGS /utf-8)
        if (PASTEQUE_ENABLE_PROFILING)
            target_link_options(${target} PRIVATE /PROFILE)
        endif ()
    else () # GCC/Clang
        if (PASTEQUE_ENABLE_PROFILING)
            target_compile_options(${target} PRIVATE -g)
        endif ()
    endif ()
endfunction()

# Sub-dependencies
if (WIN32)
    add_subdirectory(external/pdcurses)
endif ()
add_subdirectory(external/rgr)

# When creating a new C file, make sure to add it to this list!
set(SOURCE_FILES
        src/board.c src/game.c src/panel.c src/game_state.c src/colors.c src/scene.c
        src/scenes/main_menu_scene.c src/scenes/crush_scene.c src/ui.c src/highscore.c)

add_executable(projet_pasteque ${SOURCE_FILES})
add_common_compile_options(projet_pasteque)
target_link_libraries(projet_pasteque GameRGR2)
if (UNIX)
    target_link_libraries(projet_pasteque m)
endif ()
target_include_directories(projet_pasteque PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include/>)

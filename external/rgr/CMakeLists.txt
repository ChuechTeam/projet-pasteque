add_library(GameRGR2 src/error.c src/libGameRGR2.c src/utils.c)
target_include_directories(GameRGR2 PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/>
        $<INSTALL_INTERFACE:inc/>)

add_common_compile_options(GameRGR2)
if (MSVC)
    target_link_libraries(GameRGR2 PUBLIC PDCurses)
else ()
    add_library(ncursesw SHARED IMPORTED)
    target_link_libraries(GameRGR2 ncursesw)
endif ()
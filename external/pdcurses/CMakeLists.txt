add_library(PDCurses STATIC IMPORTED GLOBAL) # I spent 30 minutes figuring out you need GLOBAL

# Prefer the locally compiled version (if any) rather than the precompiled one
if (EXISTS out/pdcurses.lib)
    set_target_properties(PDCurses PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/out/pdcurses.lib)
else()
    set_target_properties(PDCurses PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/precompiled/pdcurses.lib)
endif()

# Curiously the include files are located in the root.
target_include_directories(PDCurses INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:.>)

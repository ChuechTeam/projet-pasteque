# This makefile compiles Projet Pasteque using GCC (or another compiler, if needed).
# Note that this makefile is barely optimized for incremental compilation.
# If you need incremental compilation, use CMake instead.
CC = gcc
# Add any options here
CFLAGS = -std=c11 -fPIC
# Enable multi threading
MAKEFLAGS = -j 4
OUT = build-make
INCLUDE_DIRS = include external/rgr/inc
INCLUDE_FLAGS = $(addprefix -I, $(INCLUDE_DIRS))
# Add any library here
LDLIBS = GameRGR2 ncursesw
LDFLAGS += $(addprefix -l, $(LDLIBS))
LIBRARY_PATHS = external/rgr/lib
# This is for LD_LIBRARY_PATH. Add the : infix if we have multiple libraries.
LIBRARY_PATHS_ENV = external/rgr/lib
LIBRARY_PATHS_FLAGS = $(addprefix -L, $(LIBRARY_PATHS))
# Register all C files here
SRC_FILES = board.c game.c game_state.c panel.c colors.c scene.c scenes/main_menu_scene.c scenes/crush_scene.c
OBJ_FILES = $(SRC_FILES:.c=.o)
OBJ_FILES_FP = $(addprefix $(OUT)/, $(OBJ_FILES))
# Full paths to the project includes
INCLUDE_FILES_FP = $(wildcard include/**/*.h) $(wildcard include/*.h)

all: build

.PHONY: make_build_dir
make_build_dir:
	@mkdir -p $(OUT) && mkdir -p $(OUT)/scenes

# The default rule for all object files.
$(OUT)/%.o: src/%.c $(INCLUDE_FILES_FP) external/rgr/lib/libGameRGR2.so | make_build_dir
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $(INCLUDE_FLAGS) $(LIBRARY_PATHS_FLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT)/projet_pasteque: $(OBJ_FILES_FP) 
	@echo "Linking executable..."
	@# It's necessary to have library flags at the end of the command for the linker to resolve
	@# dependencies correctly. Weird behavior. But it works.
	@$(CC) $(CFLAGS) $(INCLUDE_FLAGS) $(LIBRARY_PATHS_FLAGS)  $^ -o $@ $(LDFLAGS)

external/rgr/lib/libGameRGR2.so:
	@echo "Compiling library GameRGR2..."
	@cd ./external/rgr && mkdir -p ./lib && $(MAKE) lib/libGameRGR2.so

.PHONY: build
build: $(OUT)/projet_pasteque

.PHONY: run
run: build
	LD_LIBRARY_PATH="$(LIBRARY_PATHS_ENV)" ./$(OUT)/projet_pasteque

.PHONY: clean
clean:
	@if [ -d "$(OUT)/" ]; then rm -r $(OUT)/*; fi;
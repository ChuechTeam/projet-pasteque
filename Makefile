# This makefile compiles Projet Pasteque using GCC (or another compiler, if needed).
# Note that this makefile is barely optimized for incremental compilation.
# If you need incremental compilation, use CMake instead.
CC = gcc
# Add any options here
CFLAGS = -fPIC
# Enable multi threading
MAKEFLAGS = -j 4
OUT = build-make
INCLUDE_DIRS = include
INCLUDE_FLAGS = $(addprefix -I, $(INCLUDE_DIRS))
# Add any library here
LIBRARIES = GameRGR2 ncursesw
LIBRARIES_FLAGS = $(addprefix -l, $(LIBRARIES))
LIBRARY_PATHS = external/rgr/lib
LIBRARY_PATHS_FLAGS = $(addprefix -L, $(LIBRARY_PATHS))
# Register all C files here
SRC_FILES = main.c game.c
OBJ_FILES = $(SRC_FILES:.c=.o)
OBJ_FILES_FP = $(addprefix $(OUT)/, $(OBJ_FILES))
INCLUDE_FILES_FP = $(wildcard include/**/*.h) $(wildcard include/*.h) # Full paths to the includes

all: build

.PHONY: make_build_dir
make_build_dir:
	@mkdir -p $(OUT)

# The default rule for all object files.
$(OUT)/%.o: src/%.c $(INCLUDE_FILES_FP) external/rgr/lib/libGameRGR2.so | make_build_dir
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $(INCLUDE_FLAGS) $(LIBRARY_PATHS_FLAGS) $(LIBRARIES_FLAGS) -c $< -o $@

$(OUT)/projet_pasteque: $(OBJ_FILES_FP) 
	@echo "Linking executable..."
	@$(CC) $(CFLAGS) $(INCLUDE_FLAGS) $(LIBRARY_PATHS_FLAGS) $(LIBRARIES_FLAGS) $< -o $@

external/rgr/lib/libGameRGR2.so:
	@echo "Compiling library GameRGR2..."
	@cd ./external/rgr && mkdir -p ./lib && $(MAKE) lib/libGameRGR2.so

.PHONY: build
build: $(OUT)/projet_pasteque

.PHONY: run
run: build
	@./$(OUT)/projet_pasteque

.PHONY: clean
clean:
	@if [ -d "$(OUT)/" ]; then rm -r $(OUT)/*; fi;
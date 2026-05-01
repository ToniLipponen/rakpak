# Compiler
CXX := g++
PKG_CONFIG := pkg-config

# Project
TARGET := rakpak
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Find all source files recursively
SRC := $(shell find $(SRC_DIR) -name '*.cpp')
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# Includes
INCLUDES := -Iinclude -Iinclude/build -Iinclude/project -Iinclude/utils

# pkg-config (fmt)
FMT_CFLAGS := $(shell $(PKG_CONFIG) --cflags fmt libxxhash)
FMT_LIBS   := $(shell $(PKG_CONFIG) --libs fmt libxxhash)

# Base flags
CXXFLAGS := -std=c++17 $(INCLUDES) $(FMT_CFLAGS)
LDFLAGS  := $(FMT_LIBS)

# Default profile = release 
PROFILE ?= release 

ifeq ($(PROFILE),debug)
	CXXFLAGS += -O0 -g -DDEBUG
endif

ifeq ($(PROFILE),release)
	CXXFLAGS += -O2
endif

# Default target
all: $(BIN_DIR)/$(TARGET)

# Link
$(BIN_DIR)/$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run
run: all
	./$(BIN_DIR)/$(TARGET)

.PHONY: all clean run
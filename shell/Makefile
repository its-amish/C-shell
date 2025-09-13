# Makefile for the custom shell project
######## LLM GENERATED CODE STARTS #####


# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Source and build directories
SRC_DIR = src
OBJ_DIR = obj

# Source files (inside src/)
SRCS = main.c execute.c input.c hop.c reveal.c log.c parser.c store_logs.c \
       jobs.c lets_go.c static_run.c activites.c ping.c linkedlist.c bgfg.c

# Add paths
SRCS := $(addprefix $(SRC_DIR)/, $(SRCS))
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Executable name
TARGET = shell.out

# Default target
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compilation rule (build obj/ if missing)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create obj directory if it doesn’t exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Declare phony targets
.PHONY: all clean



######## LLM GENERATED CODE ENDS #####
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -O2 -g
CFLAGS += -fstack-protector-strong -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
CFLAGS += -I./include

# AddressSanitizer for debugging (use with: make debug or make debug-full)
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = .

# Source files
COMMON_SRC = $(wildcard $(SRC_DIR)/common/*.c)
ENGINE_SRC = $(wildcard $(SRC_DIR)/engine/*.c)
CLI_SRC = $(wildcard $(SRC_DIR)/cli/*.c)
GUI_SRC = $(wildcard $(SRC_DIR)/gui/*.c)

# Object files
COMMON_OBJ = $(COMMON_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
ENGINE_OBJ = $(ENGINE_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CLI_OBJ = $(CLI_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
GUI_OBJ = $(GUI_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Targets
CLI_BIN = $(BIN_DIR)/calc42-cli
GUI_BIN = $(BIN_DIR)/calc42-gui

# Platform detection
UNAME_S = $(shell uname -s)

# Libraries
LIBS = -lm

# Try to detect and link readline
ifeq ($(UNAME_S),Darwin)
    # macOS
    LIBS += -lreadline
else ifeq ($(UNAME_S),Linux)
    # Linux
    LIBS += -lreadline
endif

# GTK4 flags
GTK_CFLAGS = $(shell pkg-config --cflags gtk4 2>/dev/null)
GTK_LIBS = $(shell pkg-config --libs gtk4 2>/dev/null)

# Default target
all: $(CLI_BIN)

# Build both CLI and GUI
full: $(CLI_BIN) $(GUI_BIN)

# CLI binary
$(CLI_BIN): $(COMMON_OBJ) $(ENGINE_OBJ) $(CLI_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Built $(CLI_BIN)"

# GUI binary
$(GUI_BIN): $(COMMON_OBJ) $(ENGINE_OBJ) $(GUI_OBJ) | $(BIN_DIR)
	@if [ -z "$(GTK_LIBS)" ]; then \
		echo "Error: GTK4 not found. Install with: brew install gtk4 (macOS) or apt install libgtk-4-dev (Linux)"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) $(GTK_CFLAGS) $^ -o $@ $(LIBS) $(GTK_LIBS)
	@echo "Built $(GUI_BIN)"

# Object file rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@if echo "$<" | grep -q "/gui/"; then \
		if [ -z "$(GTK_CFLAGS)" ]; then \
			echo "Skipping GUI file $< (GTK4 not found)"; \
		else \
			$(CC) -Wall -Wextra -std=c11 -O2 -g $(GTK_CFLAGS) -I./include -Wno-deprecated-declarations -c $< -o $@; \
		fi; \
	else \
		$(CC) $(CFLAGS) -c $< -o $@; \
	fi

# Create directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/common $(OBJ_DIR)/engine $(OBJ_DIR)/cli $(OBJ_DIR)/gui

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean object files
clean:
	rm -rf $(OBJ_DIR)
	@echo "Objects cleaned."

# Clean objects and binaries
fclean: clean
	rm -f $(CLI_BIN) $(GUI_BIN)
	rm -f calc42.log
	@echo "Binaries and logs cleaned."

# Rebuild everything
re: fclean all

# Run CLI
run-cli: $(CLI_BIN)
	./$(CLI_BIN)

# Run GUI
run-gui: $(GUI_BIN)
	./$(GUI_BIN)

# Valgrind memory check (Linux)
valgrind: fclean $(CLI_BIN)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(CLI_BIN) "3 + 4 * 2"

# Debug with AddressSanitizer (Cross-platform: macOS/Linux)
debug: fclean
	@$(MAKE) CFLAGS="$(CFLAGS) $(ASAN_FLAGS)" all
	@echo "Built $(CLI_BIN) with AddressSanitizer."
	@echo "Running test expression..."
	./$(CLI_BIN) "mean(10, 20, 30)"

debug-full: fclean
	@$(MAKE) CFLAGS="$(CFLAGS) $(ASAN_FLAGS)" GTK_CFLAGS="$(GTK_CFLAGS) $(ASAN_FLAGS)" full
	@echo "Built $(CLI_BIN) and $(GUI_BIN) with AddressSanitizer."

# Test
test: $(CLI_BIN)
	@echo "Testing basic arithmetic..."
	@./$(CLI_BIN) "3 + 4 * 2" | grep -q "11" && echo "✓ Basic arithmetic" || echo "✗ Basic arithmetic"
	@./$(CLI_BIN) "(3 + 4) * 2" | grep -q "14" && echo "✓ Parentheses" || echo "✗ Parentheses"
	@./$(CLI_BIN) "10 / 2" | grep -q "5" && echo "✓ Division" || echo "✗ Division"
	@./$(CLI_BIN) "10 % 3" | grep -q "1" && echo "✓ Modulo" || echo "✗ Modulo"
	@echo "All tests completed!"

.PHONY: all full clean fclean re debug debug-full run-cli run-gui valgrind test

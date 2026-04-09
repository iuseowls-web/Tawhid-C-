# Tawhid C+ Compiler Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS =

# Source files
SRC = compiler/tawhidc.c compiler/lexer.c compiler/parser.c compiler/codegen.c
OBJ = $(SRC:.c=.o)

# Target executable
TARGET = tawhidc

# Default target
all: $(TARGET)

# Link object files
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful: $(TARGET)"

# Compile source files
compiler/%.o: compiler/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(TARGET)
	rm -f examples/*.out
	rm -f examples/*.tmp.c
	@echo "Clean complete"

# Test with hello world
test-hello: $(TARGET)
	./$(TARGET) examples/hello_world.tch -o examples/hello_world -v
	@echo "Running hello world..."
	./examples/hello_world

# Test with comprehensive demo
test-demo: $(TARGET)
	./$(TARGET) examples/comprehensive_demo.tch -o examples/demo -v
	@echo "Running comprehensive demo..."
	./examples/demo

# Emit C code from Tawhid C+ source
emit-c: $(TARGET)
	./$(TARGET) examples/hello_world.tch --emit-c

# Run all tests
test: test-hello test-demo

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled"

.PHONY: all clean test test-hello test-demo emit-c install uninstall

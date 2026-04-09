#!/bin/bash
# Tawhid C+ Build Script
# Simplified build process for the Tawhid C+ compiler

set -e

echo "================================"
echo "Tawhid C+ Compiler Build Script"
echo "================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if gcc is installed
if ! command -v gcc &> /dev/null; then
    echo -e "${RED}Error: gcc is not installed${NC}"
    echo "Please install gcc to compile the Tawhid C+ compiler"
    exit 1
fi

# Create build directory if it doesn't exist
BUILD_DIR="build"
mkdir -p "$BUILD_DIR"

echo -e "${YELLOW}Compiling Tawhid C+ compiler...${NC}"
echo ""

# Compile with warnings and debug info
gcc -Wall -Wextra -std=c99 -g \
    compiler/tawhidc.c \
    compiler/lexer.c \
    compiler/parser.c \
    compiler/codegen.c \
    -o tawhidc \
    -lm

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✓ Build successful!${NC}"
    echo -e "${GREEN}✓ Compiler executable: ./tawhidc${NC}"
    echo ""
    echo "Usage examples:"
    echo "  ./tawhidc examples/hello_world.tch -o hello"
    echo "  ./tawhidc examples/hello_world.tch --emit-c"
    echo "  ./tawhidc examples/hello_world.tch -v -O2"
else
    echo ""
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

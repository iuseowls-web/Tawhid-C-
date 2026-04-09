# Tawhid-C-
the holy language by god's lonely programmer and the one used to build tawhid OS
# Tawhid C+ Programming Language

A modern systems programming language inspired by C, with enhanced safety features, better error handling, and modern conveniences while maintaining C's core syntax and low-level capabilities.

## Overview

Tawhid C+ (pronounced "Tawhid C Plus") retains all the power and flexibility of C while introducing:

- **Type Inference** - Cleaner code with `:=` operator
- **Memory Safety** - Defer statements for automatic cleanup
- **Better Error Handling** - Result types and try/catch mechanism
- **Null Safety** - Optional type system with `?.` operator
- **Modern Syntax** - Range-based loops, pattern matching, and more
- **Backwards Compatible** - Nearly all valid C code works in Tawhid C+

## Quick Start

### Building the Compiler

**Prerequisites:**
- GCC compiler (version 4.9 or later)
- Make (optional, but recommended)
- Linux/Unix environment

**Using the build script:**
```bash
./build.sh
```

**Using Make:**
```bash
make clean
make all
```

This will create the `tawhidc` compiler executable.

### Your First Program

Create a file named `hello.tch`:

```tch
import stdio;

func main() -> int {
    printf("Hello, World!\n");
    return 0;
}
```

### Compiling and Running

**Compile to binary:**
```bash
./tawhidc hello.tch -o hello
./hello
```

**Generate C code:**
```bash
./tawhidc hello.tch --emit-c
```

**Verbose compilation:**
```bash
./tawhidc hello.tch -o hello -v
```

## Compiler Options

```
Usage: tawhidc [options] <input.tch>

Options:
  -o <output>    Output file name (default: a.out)
  -O <level>     Optimization level (0-3)
  -g             Generate debug information
  --emit-c       Generate C code instead of binary
  -Wall          Enable all warnings
  -v             Verbose output
  -h             Show help message
```

## Language Features

### 1. Type Inference

```tch
// Traditional C style
int x = 10;
char* name = "Alice";

// Modern style with type inference
count := 10
message := "Hello, World!"
ptr := &x
```

### 2. Memory Management with Defer

```tch
func process_data() {
    buffer := malloc(1024);
    defer free(buffer);  // Always executed when scope exits
    
    // Use buffer safely
    printf("Processing...\n");
} // buffer automatically freed here
```

### 3. Error Handling

```tch
func divide(a: int, b: int) -> result<int, string> {
    if b == 0 {
        return Err("Division by zero");
    }
    return Ok(a / b);
}

func main() -> int {
    try {
        result := try divide(10, 2);
        printf("Result: %d\n", result);
    } catch string error {
        printf("Error: %s\n", error);
    }
    return 0;
}
```

### 4. Pointer Safety

```tch
int* ptr = NULL;

// Safe null check
if (ptr != NULL) {
    printf("Value: %d\n", *ptr);
}

// Null-safe operator
value := ptr?.value ?? 0;
```

### 5. Structures with Methods

```tch
struct Person {
    string name;
    int age;
    
    func introduce(self) {
        printf("Hi, I'm %s, age %d\n", self.name, self.age);
    }
}

func main() -> int {
    person := Person("Alice", 30);
    person.introduce();
    return 0;
}
```

### 6. Enhanced Loops

```tch
// Traditional C-style
for (int i = 0; i < 10; i++) {
    printf("%d\n", i);
}

// Range-based loop
for i in 0..10 {
    printf("%d\n", i);
}

// Iterate over collection
for item in array {
    printf("%d\n", item);
}
```

### 7. Pattern Matching

```tch
match status {
    Success => printf("Success!\n");
    Failed => printf("Failed!\n");
    Pending => printf("Pending...\n");
}
```

## Project Structure

```
Tawhid C+/
├── compiler/               # Compiler source code (C)
│   ├── tawhidc.c          # Main compiler entry point
│   ├── lexer.c            # Lexical analyzer
│   ├── lexer.h            # Lexer header
│   ├── parser.c           # Parser implementation
│   ├── parser.h           # Parser header
│   ├── codegen.c          # Code generator
│   └── codegen.h          # Code generator header
├── examples/              # Example programs
│   ├── hello_world.tch    # Simple Hello World
│   └── comprehensive_demo.tch  # Full feature demo
├── stdlib/                # Standard library headers
│   ├── stdio.tchh        # I/O functions
│   └── stdlib.tchh       # General utilities
├── LANGUAGE_SPEC.md       # Complete language specification
├── Makefile              # Build configuration
├── build.sh              # Build script
└── README.md             # This file
```

## Examples

### Hello World
```bash
./tawhidc examples/hello_world.tch -o hello
./hello
```

### Comprehensive Demo
```bash
./tawhidc examples/comprehensive_demo.tch -o demo
./demo
```

### View Generated C Code
```bash
./tawhidc examples/hello_world.tch --emit-c
```

## Compilation Pipeline

```
Tawhid C+ Source (.tch)
    ↓
[Lexer] - Tokenize source code
    ↓
[Parser] - Build Abstract Syntax Tree (AST)
    ↓
[Semantic Analyzer] - Type checking
    ↓
[Code Generator] - Generate C code
    ↓
[GCC] - Compile to native binary
    ↓
Executable
```

## Differences from C

| Feature | C | Tawhid C+ |
|---------|---|-----------|
| Type Inference | No | Yes (`:=`) |
| Memory Cleanup | Manual | Defer statements |
| Error Handling | Return codes | Result types + try/catch |
| Null Safety | None | Optional types, `?.` operator |
| String Type | char* | string (safer) |
| Loops | Traditional | + Range-based |
| Pattern Matching | No | Yes (`match`) |
| Methods in Structs | No | Yes |
| Module System | #include | import statement |

## File Extensions

- `.tch` - Tawhid C+ source files
- `.tchh` - Tawhid C+ header files
- `.tchm` - Tawhid C+ module files

## Roadmap

### Phase 1: Core Language (Current)
- [x] Lexer and Parser
- [x] Basic type system
- [x] C-compatible code generation
- [x] Hello World compilation

### Phase 2: Safety Features
- [ ] Null safety checks
- [ ] Bounds checking
- [ ] Full defer implementation
- [ ] Complete error handling

### Phase 3: Modern Features
- [ ] Full type inference
- [ ] Module system
- [ ] Pattern matching
- [ ] Method syntax

### Phase 4: Optimization & Tooling
- [ ] LLVM integration
- [ ] Optimization passes
- [ ] Code formatter (tawhid-fmt)
- [ ] Linter (tawhid-lint)
- [ ] Package manager (tawhid-pkg)

## Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source. Feel free to use, modify, and distribute.

## Acknowledgments

- Inspired by C's simplicity and power
- Ideas borrowed from Rust, Go, and modern C++
- Built with GCC for maximum compatibility

## Contact & Support

For questions, issues, or suggestions:
- Check the [Language Specification](LANGUAGE_SPEC.md)
- Review example programs in `examples/`
- Study the compiler source code in `compiler/`

---

**Welcome to Tawhid C+ - Where C meets modern programming!**

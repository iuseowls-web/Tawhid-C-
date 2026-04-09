# Tawhid C+ Language Specification

## Overview
Tawhid C+ is a modern systems programming language that maintains C's simplicity and performance while addressing its most criticized weaknesses through enhanced safety features, better error handling, and modern conveniences.

## Design Philosophy
- **Backwards Compatible**: C developers should feel immediately at home
- **Safety by Default**: Catch errors at compile-time, not runtime
- **Zero-Cost Abstractions**: Modern features without performance penalty
- **Explicit over Implicit**: Maintain C's transparency while adding optional conveniences

## Core Syntax

### Lexical Structure
- Curly braces `{}` for code blocks
- Semicolons `;` for statement termination
- Comments: `//` single-line, `/* */` multi-line
- Preprocessor directives: `#include`, `#define`, `#ifdef` (enhanced)

### New Operators
- `?` - Null safety operator (e.g., `ptr?.field`)
- `!` - Force unwrap operator (e.g., `ptr!`)
- `:=` - Type inference declaration
- `->` - Return type indication
- `try/catch` - Error handling keywords
- `defer` - Cleanup statements

## Data Types

### Primitive Types (C-Compatible)
```
int, char, float, double, void
short, long, unsigned variants
bool (with true/false literals)
```

### Enhanced Types
```
int<32>, int<64>          // Explicit bit-width types
byte                      // Alias for unsigned char
string                    // Built-in string type (safer)
optional<T>               // Nullable type wrapper
result<T, E>              // Error handling type
array<T, N>               // Bounds-checked fixed array
slice<T>                  // Dynamic array view with length
```

### Type Inference
```tch
x := 42                   // int inferred
name := "Hello"           // string inferred
ptr := &x                 // int* inferred
```

## Variables & Memory Management

### Declaration Syntax
```tch
// Traditional C style (still supported)
int x = 10;
char* ptr = NULL;

// Modern style with type inference
count := 100
message := "Hello, World!"

// Explicit with safety annotations
int<64> big_num : safe    // Bounds/range checked
raw int* pointer : unsafe // Explicit unsafe marker
```

### Memory Management
```tch
// Automatic cleanup with defer
void example() {
    int* buffer = malloc(1024);
    defer free(buffer);  // Always executed when scope exits
    
    // ... use buffer ...
} // buffer automatically freed

// Ownership system (optional)
owned int* data = malloc(100);  // Owner responsible for freeing
borrow int* ref = &data;        // Borrowed reference
```

### Pointer Safety
```tch
int* ptr = NULL;
if (ptr != NULL) {
    *ptr = 10;  // Safe: compiler verifies null check
}

// Null-safe operator
int value = ptr?.value ?? 0;  // If ptr is null, use 0

// Explicit unsafe block
unsafe {
    int* raw = malloc(100);
    // Raw pointer operations allowed here
}
```

## Control Structures

### Standard Control Flow (Enhanced)
```tch
// If statement (parentheses optional)
if x > 10 {
    // ...
} else if x == 5 {
    // ...
} else {
    // ...
}

// For loop (C-style + range-based)
for (int i = 0; i < 10; i++) {
    // Traditional C loop
}

// Range-based for loop
for i in 0..10 {
    // i goes from 0 to 9
}

for item in array {
    // Iterate over collection
}

// Switch statement (no fall-through by default)
switch value {
    case 1:
        // No break needed
    case 2:
        fallthrough;  // Explicit fall-through
    case 3:
        // ...
    default:
        // ...
}
```

### Pattern Matching
```tch
match result {
    Ok(value) => printf("Success: %d\n", value);
    Err(e) => printf("Error: %s\n", e.message);
}
```

## Functions

### Function Declaration
```tch
// Traditional C style
int add(int a, int b) {
    return a + b;
}

// Modern style with explicit return type
func multiply(a: int, b: int) -> int {
    return a * b;
}

// Function with error handling
func divide(a: int, b: int) -> result<int, string> {
    if b == 0 {
        return Err("Division by zero");
    }
    return Ok(a / b);
}
```

### Function Pointers (Enhanced)
```tch
// Traditional
int (*func_ptr)(int, int) = &add;

// Modern syntax
callback: func(int, int) -> int = &add;
```

## Error Handling

### Result Type System
```tch
func read_file(path: string) -> result<string, Error> {
    FILE* f = fopen(path, "r");
    if f == NULL {
        return Err(Error::file_not_found(path));
    }
    defer fclose(f);
    
    // ... read file ...
    return Ok(content);
}
```

### Try-Catch Mechanism
```tch
try {
    data := try read_file("config.txt");
    printf("Data: %s\n", data);
} catch Error::FileNotFound(e) {
    printf("File not found: %s\n", e.path);
} catch Error::IO(e) {
    printf("IO error: %s\n", e.message);
}
```

## Structs & Type System

### Structure Definition
```tch
// C-compatible struct
struct Point {
    int x;
    int y;
}

// Enhanced struct with methods
struct Vector {
    float x;
    float y;
    float z;
    
    // Methods
    func length(self) -> float {
        return sqrt(self.x^2 + self.y^2 + self.z^2);
    }
}
```

### Enums (Enhanced)
```tch
enum Status {
    Success,
    Failed,
    Pending,
}

// Enum with values
enum HttpStatus : int {
    OK = 200,
    NotFound = 404,
    ServerError = 500,
}
```

## Modules & Imports

### Module System
```tch
// Import modules
import stdio;
import stdlib;
import math;
import "my_module.tch";

// Selective imports
import stdio { printf, scanf };

// Module definition
module my_module {
    export func public_func() {
        // Visible outside module
    }
    
    func internal_func() {
        // Module-private
    }
}
```

## Compiler Design

### Compilation Pipeline
```
Tawhid C+ Source (.tch)
    ↓
[Lexer] - Tokenize source code
    ↓
[Parser] - Build Abstract Syntax Tree (AST)
    ↓
[Semantic Analyzer] - Type checking, safety verification
    ↓
[Safety Checker] - Null checks, bounds verification
    ↓
[Optimizer] - LLVM IR optimization
    ↓
[Code Generator] - LLVM Backend
    ↓
[Linker] - Produce executable
```

### Compilation Commands
```bash
# Basic compilation
tawhidc program.tch -o program

# With optimization levels
tawhidc program.tch -O2 -o program

# Debug build
tawhidc program.tch -g -o program_debug

# Show safety warnings
tawhidc program.tch -Wall -Wextra

# Generate C code (for compatibility)
tawhidc program.tch --emit-c > output.c
```

## Key Differences from Standard C

### Safety Improvements
| Feature | C | Tawhid C+ |
|---------|---|-----------|
| Null pointers | Unchecked | Optional type system, null-safe operators |
| Buffer overflow | No bounds checking | Automatic bounds checking on arrays |
| Memory leaks | Manual tracking | Defer statements, optional ownership |
| Type safety | Weak | Stronger with inference |
| Error handling | Return codes | Result types + try/catch |

### Modern Features Added
1. Type Inference with `:=` operator
2. Built-in String Type for safer manipulation
3. Module System (better than `#include`)
4. Pattern Matching with `match` expressions
5. Range-based Loops: `for i in 0..10`
6. Defer Statements for guaranteed cleanup
7. Result Type for explicit error handling
8. Optional Safety Annotations: `safe`/`unsafe` blocks
9. Method Syntax: Functions inside structs
10. Better Enum Support: Typed enums with values

### Backwards Compatibility
- All valid C code is nearly valid Tawhid C+ (minor adjustments)
- Can include C headers directly
- C calling convention for FFI
- Optional `use_c_style` directive for strict C compatibility

## File Extensions
- Source files: `.tch` (Tawhid C+)
- Header files: `.tchh` (Tawhid C+ Header)
- Module files: `.tchm` (Tawhid C+ Module)
- Object files: `.tcho` (intermediate)

## Tooling & Ecosystem

### Developer Tools
```bash
tawhid-lint        # Code linter
tawhid-fmt         # Code formatter
tawhid-doc         # Documentation generator
tawhid-test        # Testing framework
tawhid-build       # Build system (like make)
```

### Package Manager
```bash
tawhid-pkg install stdlib
tawhid-pkg search "http client"
tawhid-pkg publish my_library
```

## Implementation Roadmap

### Phase 1: Core Language
- Lexer and Parser
- Basic type system
- C-compatible code generation
- Hello World compilation

### Phase 2: Safety Features
- Null safety checks
- Bounds checking
- Defer implementation
- Basic error handling

### Phase 3: Modern Features
- Type inference
- Module system
- Pattern matching
- Method syntax

### Phase 4: Optimization & Tooling
- LLVM integration
- Optimization passes
- Developer tools
- Standard library

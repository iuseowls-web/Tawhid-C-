/*
 * Tawhid C+ Compiler - Main Entry Point
 * A prototype compiler for the Tawhid C+ programming language
 * Written in C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

void print_usage(const char *program_name) {
    printf("Usage: %s [options] <input.tch>\n", program_name);
    printf("\nOptions:\n");
    printf("  -o <output>    Output file name (default: a.out)\n");
    printf("  -O <level>     Optimization level (0-3)\n");
    printf("  -g             Generate debug information\n");
    printf("  --emit-c       Generate C code instead of binary\n");
    printf("  -Wall          Enable all warnings\n");
    printf("  -v             Verbose output\n");
    printf("  -h             Show this help message\n");
}

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = "a.out";
    int optimization_level = 0;
    int debug_info = 0;
    int emit_c = 0;
    int verbose = 0;
    int wall_warnings = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-O") == 0 && i + 1 < argc) {
            optimization_level = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-g") == 0) {
            debug_info = 1;
        } else if (strcmp(argv[i], "--emit-c") == 0) {
            emit_c = 1;
        } else if (strcmp(argv[i], "-Wall") == 0) {
            wall_warnings = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!input_file) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Check if input file exists
    FILE *test = fopen(input_file, "r");
    if (!test) {
        fprintf(stderr, "Error: File '%s' not found\n", input_file);
        return 1;
    }
    fclose(test);
    
    if (verbose) {
        printf("Compiling %s...\n", input_file);
    }
    
    // Read source code
    FILE *file = fopen(input_file, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", input_file);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long source_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *source_code = (char *)malloc(source_size + 1);
    if (!source_code) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    size_t read_size = fread(source_code, 1, source_size, file);
    source_code[read_size] = '\0';
    fclose(file);
    
    // Phase 1: Lexical Analysis
    if (verbose) {
        printf("Phase 1: Lexical Analysis...\n");
    }
    
    Token *tokens = NULL;
    int token_count = 0;
    int lex_result = tokenize(source_code, &tokens, &token_count);
    
    if (lex_result != 0) {
        fprintf(stderr, "Error: Lexical analysis failed\n");
        free(source_code);
        return 1;
    }
    
    if (verbose) {
        printf("  Generated %d tokens\n", token_count);
    }
    
    // Phase 2: Parsing
    if (verbose) {
        printf("Phase 2: Parsing...\n");
    }
    
    ASTNode *ast = NULL;
    int parse_result = parse(tokens, token_count, &ast);
    
    if (parse_result != 0) {
        fprintf(stderr, "Error: Parsing failed\n");
        free(source_code);
        free_tokens(tokens, token_count);
        return 1;
    }
    
    if (verbose) {
        printf("  AST generated successfully\n");
    }
    
    // Phase 3: Code Generation
    if (verbose) {
        printf("Phase 3: Code Generation...\n");
    }
    
    char *c_code = generate_c_code(ast);
    
    if (!c_code) {
        fprintf(stderr, "Error: Code generation failed\n");
        free(source_code);
        free_tokens(tokens, token_count);
        free_ast(ast);
        return 1;
    }
    
    if (emit_c) {
        // Output C code to stdout
        printf("%s\n", c_code);
        if (verbose) {
            printf("  C code generated successfully\n");
        }
    } else {
        // Write temporary C file
        char temp_file[1024];
        strncpy(temp_file, input_file, sizeof(temp_file) - 1);
        temp_file[sizeof(temp_file) - 1] = '\0';
        
        // Replace .tch extension with .tmp.c
        char *ext = strstr(temp_file, ".tch");
        if (ext) {
            strcpy(ext, ".tmp.c");
        } else {
            strcat(temp_file, ".tmp.c");
        }
        
        FILE *temp = fopen(temp_file, "w");
        if (!temp) {
            fprintf(stderr, "Error: Cannot create temporary file\n");
            free(source_code);
            free_tokens(tokens, token_count);
            free_ast(ast);
            free(c_code);
            return 1;
        }
        
        fprintf(temp, "%s\n", c_code);
        fclose(temp);
        
        // Compile with GCC
        char compile_cmd[2048];
        snprintf(compile_cmd, sizeof(compile_cmd), 
                "gcc %s -o %s", temp_file, output_file);
        
        if (optimization_level > 0) {
            char opt_flag[10];
            snprintf(opt_flag, sizeof(opt_flag), " -O%d", optimization_level);
            strncat(compile_cmd, opt_flag, sizeof(compile_cmd) - strlen(compile_cmd) - 1);
        }
        
        if (debug_info) {
            strncat(compile_cmd, " -g", sizeof(compile_cmd) - strlen(compile_cmd) - 1);
        }
        
        if (verbose) {
            printf("  Compiling with GCC: %s\n", compile_cmd);
        }
        
        int compile_result = system(compile_cmd);
        
        // Clean up temporary file
        remove(temp_file);
        
        if (compile_result != 0) {
            fprintf(stderr, "Error during compilation\n");
            free(source_code);
            free_tokens(tokens, token_count);
            free_ast(ast);
            free(c_code);
            return 1;
        }
        
        if (verbose) {
            printf("  Binary generated: %s\n", output_file);
        }
    }
    
    printf("Compilation successful!\n");
    
    // Cleanup
    free(source_code);
    free_tokens(tokens, token_count);
    free_ast(ast);
    free(c_code);
    
    return 0;
}

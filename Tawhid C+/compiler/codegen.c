/*
 * Tawhid C+ Code Generator Implementation
 * Converts AST to C code
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

typedef struct {
    char *code;
    int length;
    int capacity;
    int indent_level;
} CodeBuffer;

// Forward declarations
static void generate_node(CodeBuffer *buffer, ASTNode *node);
static void generate_call(CodeBuffer *buffer, ASTNode *node);
static void generate_variable_decl(CodeBuffer *buffer, ASTNode *node);

static void buffer_init(CodeBuffer *buffer) {
    buffer->capacity = 4096;
    buffer->length = 0;
    buffer->code = (char*)malloc(buffer->capacity);
    buffer->code[0] = '\0';
    buffer->indent_level = 0;
}

static void buffer_free(CodeBuffer *buffer) {
    if (buffer->code) {
        free(buffer->code);
    }
}

static void buffer_append(CodeBuffer *buffer, const char *text) {
    if (!text) return;
    
    int text_len = strlen(text);
    int needed = buffer->length + text_len + 1;
    
    if (needed > buffer->capacity) {
        while (buffer->capacity < needed) {
            buffer->capacity *= 2;
        }
        buffer->code = (char*)realloc(buffer->code, buffer->capacity);
    }
    
    strcpy(&buffer->code[buffer->length], text);
    buffer->length += text_len;
}

static void buffer_append_indent(CodeBuffer *buffer) {
    for (int i = 0; i < buffer->indent_level; i++) {
        buffer_append(buffer, "    ");
    }
}

static void buffer_append_line(CodeBuffer *buffer, const char *text) {
    buffer_append_indent(buffer);
    buffer_append(buffer, text);
    buffer_append(buffer, "\n");
}

static const char* get_type_name(const char *tch_type) {
    if (strcmp(tch_type, "int") == 0) return "int";
    if (strcmp(tch_type, "float") == 0) return "float";
    if (strcmp(tch_type, "double") == 0) return "double";
    if (strcmp(tch_type, "char") == 0) return "char";
    if (strcmp(tch_type, "void") == 0) return "void";
    if (strcmp(tch_type, "bool") == 0) return "int";
    if (strcmp(tch_type, "string") == 0) return "char*";
    return tch_type;
}

static void generate_node(CodeBuffer *buffer, ASTNode *node);

static void generate_literal(CodeBuffer *buffer, ASTNode *node) {
    if (node->value) {
        // Check if it's a string literal
        int is_string = 0;
        if (node->value[0] != '\0') {
            // If it doesn't start with a digit or minus sign, treat as string
            if (!(node->value[0] >= '0' && node->value[0] <= '9') && node->value[0] != '-') {
                is_string = 1;
            }
        }
        
        if (is_string) {
            buffer_append(buffer, "\"");
            // Escape special characters
            for (int i = 0; node->value[i] != '\0'; i++) {
                char c = node->value[i];
                switch (c) {
                    case '\n': buffer_append(buffer, "\\n"); break;
                    case '\t': buffer_append(buffer, "\\t"); break;
                    case '\\': buffer_append(buffer, "\\\\"); break;
                    case '"': buffer_append(buffer, "\\\""); break;
                    default: buffer_append(buffer, &c); break;
                }
            }
            buffer_append(buffer, "\"");
        } else {
            buffer_append(buffer, node->value);
        }
    }
}

static void generate_identifier(CodeBuffer *buffer, ASTNode *node) {
    if (node->value) {
        buffer_append(buffer, node->value);
    }
}

static void generate_binary_op(CodeBuffer *buffer, ASTNode *node) {
    if (node->child_count >= 2) {
        buffer_append(buffer, "(");
        generate_node(buffer, node->children[0]);
        buffer_append(buffer, " + ");
        generate_node(buffer, node->children[1]);
        buffer_append(buffer, ")");
    }
}

static void generate_function(CodeBuffer *buffer, ASTNode *node) {
    if (!node) return;
    
    // Function signature
    buffer_append_indent(buffer);
    
    // Return type (default to int if not specified)
    char *return_type = "int";
    int param_count = 0;
    int body_index = -1;
    
    // Find return type and body
    for (int i = 0; i < node->child_count; i++) {
        if (!node->children[i]) continue;  // Skip NULL children
        
        if (node->children[i]->type == AST_IDENTIFIER && i == 0) {
            // This might be return type if there are parameters
            if (node->child_count > 2 && node->children[i]->value) {
                return_type = get_type_name(node->children[i]->value);
            }
        } else if (node->children[i]->type == AST_BLOCK && 
                   node->children[i]->value &&
                   strcmp(node->children[i]->value, "parameters") == 0) {
            param_count = node->children[i]->child_count;
        } else if (node->children[i]->type == AST_BLOCK) {
            body_index = i;
        }
    }
    
    buffer_append(buffer, return_type);
    buffer_append(buffer, " ");
    buffer_append(buffer, node->value);
    buffer_append(buffer, "(");
    
    // Parameters
    int param_idx = 0;
    for (int i = 0; i < node->child_count; i++) {
        if (!node->children[i]) continue;
        if (node->children[i]->type == AST_BLOCK && 
            node->children[i]->value &&
            strcmp(node->children[i]->value, "parameters") == 0) {
            
            for (int j = 0; j < node->children[i]->child_count; j++) {
                if (param_idx > 0) {
                    buffer_append(buffer, ", ");
                }
                
                ASTNode *param = node->children[i]->children[j];
                buffer_append(buffer, param->value);
                
                // Parameter type
                if (param->child_count > 0) {
                    buffer_append(buffer, " /* ");
                    buffer_append(buffer, param->children[0]->value);
                    buffer_append(buffer, " */");
                }
                
                param_idx++;
            }
        }
    }
    
    if (param_idx == 0) {
        buffer_append(buffer, "void");
    }
    
    buffer_append(buffer, ") {\n");
    
    // Function body
    if (body_index >= 0 && body_index < node->child_count) {
        ASTNode *body = node->children[body_index];
        if (body && body->type == AST_BLOCK) {
            buffer->indent_level++;
            for (int i = 0; i < body->child_count; i++) {
                if (body->children[i]) {
                    ASTNode *child = body->children[i];
                    buffer_append_indent(buffer);
                    
                    // Generate the statement
                    if (child->type == AST_RETURN) {
                        buffer_append(buffer, "return");
                        if (child->child_count > 0 && child->children[0]) {
                            buffer_append(buffer, " ");
                            generate_node(buffer, child->children[0]);
                        }
                        buffer_append(buffer, ";\n");
                    } else if (child->type == AST_CALL) {
                        generate_call(buffer, child);
                        buffer_append(buffer, ";\n");
                    } else if (child->type == AST_VARIABLE_DECL) {
                        generate_variable_decl(buffer, child);
                        buffer_append(buffer, "\n");
                    } else {
                        generate_node(buffer, child);
                        buffer_append(buffer, ";\n");
                    }
                }
            }
            buffer->indent_level--;
            buffer_append_indent(buffer);
            buffer_append(buffer, "}\n");
        }
    } else {
        // Empty function body  
        buffer_append_indent(buffer);
        buffer_append(buffer, "}\n");
    }
}

static void generate_variable_decl(CodeBuffer *buffer, ASTNode *node) {
    buffer_append_indent(buffer);
    
    // If has type child
    if (node->child_count > 0 && node->children[0]->type == AST_IDENTIFIER) {
        buffer_append(buffer, get_type_name(node->children[0]->value));
        buffer_append(buffer, " ");
        buffer_append(buffer, node->value);
        
        if (node->child_count > 1) {
            buffer_append(buffer, " = ");
            generate_node(buffer, node->children[1]);
        }
    } else {
        // Type inference - default to int
        buffer_append(buffer, "int ");
        buffer_append(buffer, node->value);
        
        if (node->child_count > 0) {
            buffer_append(buffer, " = ");
            generate_node(buffer, node->children[0]);
        }
    }
    
    buffer_append(buffer, ";");
}

static void generate_call(CodeBuffer *buffer, ASTNode *node) {
    if (node->child_count > 0) {
        // Function name
        generate_node(buffer, node->children[0]);
        buffer_append(buffer, "(");
        
        // Arguments
        for (int i = 1; i < node->child_count; i++) {
            if (i > 1) {
                buffer_append(buffer, ", ");
            }
            generate_node(buffer, node->children[i]);
        }
        
        buffer_append(buffer, ")");
    }
}

static void generate_return(CodeBuffer *buffer, ASTNode *node) {
    buffer_append_indent(buffer);
    buffer_append(buffer, "return");
    
    if (node->child_count > 0) {
        buffer_append(buffer, " ");
        generate_node(buffer, node->children[0]);
    }
    
    buffer_append(buffer, ";");
}

static void generate_if(CodeBuffer *buffer, ASTNode *node) {
    buffer_append_indent(buffer);
    buffer_append(buffer, "if (");
    
    if (node->child_count > 0) {
        generate_node(buffer, node->children[0]);
    }
    
    buffer_append(buffer, ") {\n");
    
    // Then block
    if (node->child_count > 1) {
        buffer->indent_level++;
        ASTNode *then_block = node->children[1];
        for (int i = 0; i < then_block->child_count; i++) {
            generate_node(buffer, then_block->children[i]);
            buffer_append(buffer, "\n");
        }
        buffer->indent_level--;
    }
    
    buffer_append_indent(buffer);
    buffer_append(buffer, "}");
    
    // Optional else
    if (node->child_count > 2) {
        buffer_append(buffer, " else {\n");
        buffer->indent_level++;
        ASTNode *else_block = node->children[2];
        if (else_block->type == AST_IF) {
            buffer->indent_level--;
            buffer_append_indent(buffer);
            generate_if(buffer, else_block);
        } else {
            for (int i = 0; i < else_block->child_count; i++) {
                generate_node(buffer, else_block->children[i]);
                buffer_append(buffer, "\n");
            }
            buffer->indent_level--;
            buffer_append_indent(buffer);
            buffer_append(buffer, "}");
        }
    }
    
    buffer_append(buffer, "\n");
}

static void generate_import(CodeBuffer *buffer, ASTNode *node) {
    // Imports are handled by adding appropriate #include
    if (node->value) {
        if (strcmp(node->value, "stdio") == 0) {
            buffer_append_line(buffer, "#include <stdio.h>");
        } else if (strcmp(node->value, "stdlib") == 0) {
            buffer_append_line(buffer, "#include <stdlib.h>");
        } else if (strcmp(node->value, "string") == 0) {
            buffer_append_line(buffer, "#include <string.h>");
        } else {
            buffer_append(buffer, "#include \"");
            buffer_append(buffer, node->value);
            buffer_append_line(buffer, ".h\"");
        }
    }
}

static void generate_statement(CodeBuffer *buffer, ASTNode *node) {
    switch (node->type) {
        case AST_IMPORT:
            generate_import(buffer, node);
            break;
        case AST_FUNCTION:
            generate_function(buffer, node);
            break;
        case AST_VARIABLE_DECL:
            generate_variable_decl(buffer, node);
            break;
        case AST_RETURN:
            generate_return(buffer, node);
            break;
        case AST_IF:
            generate_if(buffer, node);
            break;
        case AST_CALL:
            buffer_append_indent(buffer);
            generate_call(buffer, node);
            buffer_append(buffer, ";");
            break;
        case AST_LITERAL:
            generate_literal(buffer, node);
            break;
        case AST_IDENTIFIER:
            generate_identifier(buffer, node);
            break;
        default:
            break;
    }
}

static void generate_node(CodeBuffer *buffer, ASTNode *node) {
    if (!node) return;
    generate_statement(buffer, node);
}

char* generate_c_code(ASTNode *ast) {
    if (!ast) return NULL;
    
    CodeBuffer buffer;
    buffer_init(&buffer);
    
    // Add header
    buffer_append_line(&buffer, "/* Generated C code from Tawhid C+ source */");
    buffer_append_line(&buffer, "#include <stdio.h>");
    buffer_append_line(&buffer, "#include <stdlib.h>");
    buffer_append_line(&buffer, "");
    
    // Generate all nodes
    for (int i = 0; i < ast->child_count; i++) {
        if (ast->children[i]) {
            generate_node(&buffer, ast->children[i]);
            buffer_append(&buffer, "\n");
        }
    }
    
    // Return the generated code
    char *result = strdup(buffer.code);
    buffer_free(&buffer);
    
    return result;
}

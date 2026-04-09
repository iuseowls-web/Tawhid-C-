/*
 * Tawhid C+ Parser Implementation
 * Builds Abstract Syntax Tree from tokens
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

typedef struct {
    Token *tokens;
    int pos;
    int token_count;
    ASTNode *ast;
} Parser;

static ASTNode* create_node(ASTNodeType type, int line, int column) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->line = line;
    node->column = column;
    return node;
}

static void add_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return;
    
    parent->child_count++;
    if (parent->children) {
        parent->children = (ASTNode**)realloc(parent->children, 
                                             sizeof(ASTNode*) * parent->child_count);
    } else {
        parent->children = (ASTNode**)malloc(sizeof(ASTNode*));
    }
    parent->children[parent->child_count - 1] = child;
}

static void set_value(ASTNode *node, const char *value) {
    if (!node || !value) return;
    node->value = strdup(value);
}

static Token* current(Parser *parser) {
    if (parser->pos >= 0 && parser->pos < parser->token_count) {
        return &parser->tokens[parser->pos];
    }
    // Return EOF token if out of bounds
    return &parser->tokens[parser->token_count - 1];
}

static Token* peek(Parser *parser, int offset) {
    int pos = parser->pos + offset;
    if (pos >= 0 && pos < parser->token_count) {
        return &parser->tokens[pos];
    }
    return &parser->tokens[parser->token_count - 1];
}

static void advance(Parser *parser) {
    if (parser->pos < parser->token_count - 1) {
        parser->pos++;
    }
}

static int expect(Parser *parser, TokenType type, const char *error_msg) {
    if (current(parser)->type == type) {
        advance(parser);
        return 1;
    }
    fprintf(stderr, "Parse error at line %d: %s\n", current(parser)->line, error_msg);
    return 0;
}

static int match(Parser *parser, TokenType type) {
    if (current(parser)->type == type) {
        advance(parser);
        return 1;
    }
    return 0;
}

// Forward declarations
static ASTNode* parse_statement(Parser *parser);
static ASTNode* parse_expression(Parser *parser);

static ASTNode* parse_import(Parser *parser) {
    advance(parser); // Skip 'import' keyword
    
    ASTNode *import_node = create_node(AST_IMPORT, current(parser)->line, current(parser)->column);
    
    // Get module name
    if (current(parser)->type == TOKEN_IDENTIFIER || current(parser)->type == TOKEN_STRING) {
        set_value(import_node, current(parser)->value);
        advance(parser);
    }
    
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after import");
    
    return import_node;
}

static ASTNode* parse_function(Parser *parser) {
    advance(parser); // Skip 'func' keyword
    
    if (parser->pos >= parser->token_count) {
        fprintf(stderr, "Parse error: Unexpected end of input\n");
        return NULL;
    }
    
    ASTNode *func_node = create_node(AST_FUNCTION, current(parser)->line, current(parser)->column);
    
    // Function name
    if (current(parser)->type == TOKEN_IDENTIFIER) {
        set_value(func_node, current(parser)->value);
        advance(parser);
    }
    
    // Parameters
    if (!expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name")) {
        return func_node;
    }
    
    ASTNode *params_node = create_node(AST_BLOCK, current(parser)->line, current(parser)->column);
    set_value(params_node, "parameters");
    
    while (current(parser)->type != TOKEN_RIGHT_PAREN && current(parser)->type != TOKEN_EOF) {
        if (current(parser)->type == TOKEN_IDENTIFIER) {
            ASTNode *param = create_node(AST_PARAMETER, current(parser)->line, current(parser)->column);
            set_value(param, current(parser)->value);
            advance(parser);
            
            // Optional type annotation
            if (match(parser, TOKEN_COLON)) {
                if (current(parser)->type == TOKEN_IDENTIFIER || 
                    current(parser)->type == TOKEN_INT ||
                    current(parser)->type == TOKEN_FLOAT_TYPE) {
                    ASTNode *type_node = create_node(AST_IDENTIFIER, current(parser)->line, current(parser)->column);
                    set_value(type_node, current(parser)->value);
                    add_child(param, type_node);
                    advance(parser);
                }
            }
            
            add_child(params_node, param);
        }
        
        if (!match(parser, TOKEN_COMMA)) break;
    }
    
    add_child(func_node, params_node);
    expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters");
    
    // Optional return type
    if (match(parser, TOKEN_ARROW)) {
        if (current(parser)->type == TOKEN_IDENTIFIER || 
            current(parser)->type == TOKEN_INT ||
            current(parser)->type == TOKEN_VOID) {
            ASTNode *return_type = create_node(AST_IDENTIFIER, current(parser)->line, current(parser)->column);
            set_value(return_type, current(parser)->value);
            add_child(func_node, return_type);
            advance(parser);
        }
    }
    
    // Function body
    if (current(parser)->type == TOKEN_LEFT_BRACE) {
        advance(parser); // Skip '{'
        
        ASTNode *body_node = create_node(AST_BLOCK, current(parser)->line, current(parser)->column);
        
        while (current(parser)->type != TOKEN_RIGHT_BRACE && current(parser)->type != TOKEN_EOF) {
            if (parser->pos >= parser->token_count) break;
            ASTNode *stmt = parse_statement(parser);
            if (stmt) {
                add_child(body_node, stmt);
            }
            if (parser->pos >= parser->token_count) break;
        }
        
        add_child(func_node, body_node);
        expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' at end of function");
    }
    
    return func_node;
}

static ASTNode* parse_variable_declaration(Parser *parser) {
    ASTNode *decl_node = create_node(AST_VARIABLE_DECL, current(parser)->line, current(parser)->column);
    
    int is_inference = 0;
    
    // Check for type inference :=
    if (current(parser)->type == TOKEN_IDENTIFIER) {
        // Could be: type name = value  OR  name := value
        if (peek(parser, 1)->type == TOKEN_COLON_ASSIGN) {
            // Type inference: name := value
            set_value(decl_node, current(parser)->value);
            is_inference = 1;
            advance(parser);
            advance(parser); // Skip ':='
        } else if (peek(parser, 1)->type == TOKEN_IDENTIFIER) {
            // Traditional: type name = value
            ASTNode *type_node = create_node(AST_IDENTIFIER, current(parser)->line, current(parser)->column);
            set_value(type_node, current(parser)->value);
            add_child(decl_node, type_node);
            advance(parser);
            
            set_value(decl_node, current(parser)->value);
            advance(parser);
            
            expect(parser, TOKEN_ASSIGN, "Expected '=' in variable declaration");
        }
    }
    
    // Parse initial value
    ASTNode *value = parse_expression(parser);
    if (value) {
        add_child(decl_node, value);
    }
    
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration");
    
    return decl_node;
}

static ASTNode* parse_if_statement(Parser *parser) {
    advance(parser); // Skip 'if'
    
    ASTNode *if_node = create_node(AST_IF, current(parser)->line, current(parser)->column);
    
    // Optional parenthesis
    match(parser, TOKEN_LEFT_PAREN);
    
    // Condition
    ASTNode *condition = parse_expression(parser);
    if (condition) {
        add_child(if_node, condition);
    }
    
    match(parser, TOKEN_RIGHT_PAREN);
    
    // Then block
    if (current(parser)->type == TOKEN_LEFT_BRACE) {
        advance(parser);
        
        ASTNode *then_block = create_node(AST_BLOCK, current(parser)->line, current(parser)->column);
        while (current(parser)->type != TOKEN_RIGHT_BRACE && current(parser)->type != TOKEN_EOF) {
            ASTNode *stmt = parse_statement(parser);
            if (stmt) add_child(then_block, stmt);
        }
        add_child(if_node, then_block);
        expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after if block");
    }
    
    // Optional else
    if (match(parser, TOKEN_ELSE)) {
        if (current(parser)->type == TOKEN_IF) {
            // else if
            ASTNode *else_if = parse_if_statement(parser);
            if (else_if) add_child(if_node, else_if);
        } else if (current(parser)->type == TOKEN_LEFT_BRACE) {
            advance(parser);
            
            ASTNode *else_block = create_node(AST_BLOCK, current(parser)->line, current(parser)->column);
            while (current(parser)->type != TOKEN_RIGHT_BRACE && current(parser)->type != TOKEN_EOF) {
                ASTNode *stmt = parse_statement(parser);
                if (stmt) add_child(else_block, stmt);
            }
            add_child(if_node, else_block);
            expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after else block");
        }
    }
    
    return if_node;
}

static ASTNode* parse_for_loop(Parser *parser) {
    advance(parser); // Skip 'for'
    
    ASTNode *for_node = create_node(AST_FOR, current(parser)->line, current(parser)->column);
    
    // Check if it's a range-based for: for x in range
    if (current(parser)->type == TOKEN_IDENTIFIER) {
        ASTNode *var = create_node(AST_IDENTIFIER, current(parser)->line, current(parser)->column);
        set_value(var, current(parser)->value);
        add_child(for_node, var);
        advance(parser);
        
        if (match(parser, TOKEN_IN)) {
            // Range-based for
            ASTNode *range = parse_expression(parser);
            if (range) add_child(for_node, range);
        }
    } else {
        // Traditional C-style for loop
        expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'for'");
        
        // Initialization
        if (current(parser)->type != TOKEN_SEMICOLON) {
            ASTNode *init = parse_statement(parser);
            if (init) add_child(for_node, init);
        } else {
            advance(parser);
        }
        
        // Condition
        if (current(parser)->type != TOKEN_SEMICOLON) {
            ASTNode *condition = parse_expression(parser);
            if (condition) add_child(for_node, condition);
        }
        expect(parser, TOKEN_SEMICOLON, "Expected ';' in for loop");
        
        // Increment
        if (current(parser)->type != TOKEN_RIGHT_PAREN) {
            ASTNode *increment = parse_expression(parser);
            if (increment) add_child(for_node, increment);
        }
        
        expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' in for loop");
    }
    
    // Loop body
    if (current(parser)->type == TOKEN_LEFT_BRACE) {
        advance(parser);
        
        ASTNode *body = create_node(AST_BLOCK, current(parser)->line, current(parser)->column);
        while (current(parser)->type != TOKEN_RIGHT_BRACE && current(parser)->type != TOKEN_EOF) {
            ASTNode *stmt = parse_statement(parser);
            if (stmt) add_child(body, stmt);
        }
        add_child(for_node, body);
        expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after for loop body");
    }
    
    return for_node;
}

static ASTNode* parse_return_statement(Parser *parser) {
    advance(parser); // Skip 'return'
    
    ASTNode *return_node = create_node(AST_RETURN, current(parser)->line, current(parser)->column);
    
    if (current(parser)->type != TOKEN_SEMICOLON) {
        ASTNode *value = parse_expression(parser);
        if (value) add_child(return_node, value);
    }
    
    expect(parser, TOKEN_SEMICOLON, "Expected ';' after return");
    
    return return_node;
}

static ASTNode* parse_call_expression(Parser *parser, ASTNode *callee) {
    ASTNode *call_node = create_node(AST_CALL, current(parser)->line, current(parser)->column);
    add_child(call_node, callee);
    
    expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name");
    
    while (current(parser)->type != TOKEN_RIGHT_PAREN && current(parser)->type != TOKEN_EOF) {
        ASTNode *arg = parse_expression(parser);
        if (arg) add_child(call_node, arg);
        
        if (!match(parser, TOKEN_COMMA)) break;
    }
    
    expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after function arguments");
    
    return call_node;
}

static ASTNode* parse_primary(Parser *parser) {
    if (current(parser)->type == TOKEN_INTEGER) {
        ASTNode *node = create_node(AST_LITERAL, current(parser)->line, current(parser)->column);
        set_value(node, current(parser)->value);
        advance(parser);
        return node;
    }
    
    if (current(parser)->type == TOKEN_STRING) {
        ASTNode *node = create_node(AST_LITERAL, current(parser)->line, current(parser)->column);
        set_value(node, current(parser)->value);
        advance(parser);
        return node;
    }
    
    if (current(parser)->type == TOKEN_IDENTIFIER) {
        ASTNode *node = create_node(AST_IDENTIFIER, current(parser)->line, current(parser)->column);
        set_value(node, current(parser)->value);
        advance(parser);
        
        // Check if it's a function call
        if (current(parser)->type == TOKEN_LEFT_PAREN) {
            return parse_call_expression(parser, node);
        }
        
        return node;
    }
    
    if (match(parser, TOKEN_LEFT_PAREN)) {
        ASTNode *expr = parse_expression(parser);
        expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    fprintf(stderr, "Parse error at line %d: Unexpected token\n", current(parser)->line);
    return NULL;
}

static ASTNode* parse_expression(Parser *parser) {
    return parse_primary(parser);
}

static ASTNode* parse_statement(Parser *parser) {
    if (current(parser)->type == TOKEN_IMPORT) {
        return parse_import(parser);
    }
    
    if (current(parser)->type == TOKEN_FUNC) {
        return parse_function(parser);
    }
    
    if (current(parser)->type == TOKEN_IF) {
        return parse_if_statement(parser);
    }
    
    if (current(parser)->type == TOKEN_FOR) {
        return parse_for_loop(parser);
    }
    
    if (current(parser)->type == TOKEN_RETURN) {
        return parse_return_statement(parser);
    }
    
    // Variable declaration or expression statement
    if (current(parser)->type == TOKEN_IDENTIFIER) {
        // Check if next token exists before peeking
        if (parser->pos + 1 < parser->token_count) {
            if (peek(parser, 1)->type == TOKEN_COLON_ASSIGN || 
                (peek(parser, 1)->type == TOKEN_IDENTIFIER && 
                 parser->pos + 2 < parser->token_count &&
                 peek(parser, 2)->type == TOKEN_ASSIGN)) {
                return parse_variable_declaration(parser);
            }
        }
    }
    
    // Expression statement
    ASTNode *expr = parse_expression(parser);
    if (expr) {
        expect(parser, TOKEN_SEMICOLON, "Expected ';' after statement");
    }
    
    return expr;
}

int parse(Token *tokens, int token_count, ASTNode **ast) {
    if (!tokens || token_count == 0 || !ast) {
        fprintf(stderr, "Parse error: Invalid input\n");
        return 1;
    }
    
    Parser parser;
    parser.tokens = tokens;
    parser.pos = 0;
    parser.token_count = token_count;
    parser.ast = NULL;
    
    // Create root program node
    ASTNode *program = create_node(AST_PROGRAM, 0, 0);
    set_value(program, "program");
    
    // Parse all top-level statements
    while (current(&parser)->type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement(&parser);
        if (stmt) {
            add_child(program, stmt);
        }
    }
    
    *ast = program;
    return 0;
}

void free_ast(ASTNode *node) {
    if (!node) return;
    
    if (node->children) {
        for (int i = 0; i < node->child_count; i++) {
            free_ast(node->children[i]);
        }
        free(node->children);
    }
    
    if (node->value) {
        free(node->value);
    }
    
    free(node);
}

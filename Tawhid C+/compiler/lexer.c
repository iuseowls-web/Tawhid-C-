/*
 * Tawhid C+ Lexer Implementation
 * Tokenizes source code into tokens
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

typedef struct {
    const char *source;
    int pos;
    int line;
    int column;
    Token *tokens;
    int token_count;
    int token_capacity;
} Lexer;

static const char* KEYWORDS[] = {
    "func", "import", "struct", "enum", "if", "else", "for", "while",
    "return", "int", "float", "double", "char", "void", "bool",
    "true", "false", "string", "try", "catch", "defer", "match",
    "unsafe", "module", "export", "in"
};

static TokenType KEYWORD_TYPES[] = {
    TOKEN_FUNC, TOKEN_IMPORT, TOKEN_STRUCT, TOKEN_ENUM, TOKEN_IF, TOKEN_ELSE,
    TOKEN_FOR, TOKEN_WHILE, TOKEN_RETURN, TOKEN_INT, TOKEN_FLOAT_TYPE,
    TOKEN_DOUBLE, TOKEN_CHAR, TOKEN_VOID, TOKEN_BOOL, TOKEN_TRUE, TOKEN_FALSE,
    TOKEN_STRING_TYPE, TOKEN_TRY, TOKEN_CATCH, TOKEN_DEFER, TOKEN_MATCH,
    TOKEN_UNSAFE, TOKEN_MODULE, TOKEN_EXPORT, TOKEN_IN
};

static int NUM_KEYWORDS = 26;

static void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->token_count = 0;
    lexer->token_capacity = 1024;
    lexer->tokens = (Token*)malloc(sizeof(Token) * lexer->token_capacity);
}

static void lexer_free(Lexer *lexer) {
    if (lexer->tokens) {
        for (int i = 0; i < lexer->token_count; i++) {
            if (lexer->tokens[i].value) {
                free(lexer->tokens[i].value);
            }
        }
        free(lexer->tokens);
    }
}

static char* lexer_error(Lexer *lexer, const char *message) {
    fprintf(stderr, "Lexer error at line %d, column %d: %s\n", 
            lexer->line, lexer->column, message);
    return NULL;
}

static void advance(Lexer *lexer) {
    if (lexer->source[lexer->pos] != '\0') {
        if (lexer->source[lexer->pos] == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->pos++;
    }
}

static char peek(Lexer *lexer) {
    return lexer->source[lexer->pos];
}

static char peek_next(Lexer *lexer) {
    if (lexer->source[lexer->pos + 1] != '\0') {
        return lexer->source[lexer->pos + 1];
    }
    return '\0';
}

static void add_token(Lexer *lexer, TokenType type, const char *value, int start_line, int start_col) {
    if (lexer->token_count >= lexer->token_capacity) {
        lexer->token_capacity *= 2;
        lexer->tokens = (Token*)realloc(lexer->tokens, sizeof(Token) * lexer->token_capacity);
    }
    
    lexer->tokens[lexer->token_count].type = type;
    lexer->tokens[lexer->token_count].value = value ? strdup(value) : NULL;
    lexer->tokens[lexer->token_count].line = start_line;
    lexer->tokens[lexer->token_count].column = start_col;
    lexer->token_count++;
}

static void skip_whitespace(Lexer *lexer) {
    while (lexer->source[lexer->pos] != '\0' && 
           (lexer->source[lexer->pos] == ' ' || 
            lexer->source[lexer->pos] == '\t' ||
            lexer->source[lexer->pos] == '\n' ||
            lexer->source[lexer->pos] == '\r')) {
        advance(lexer);
    }
}

static int skip_comment(Lexer *lexer) {
    if (peek(lexer) == '/') {
        if (peek_next(lexer) == '/') {
            // Single line comment
            while (lexer->source[lexer->pos] != '\0' && lexer->source[lexer->pos] != '\n') {
                advance(lexer);
            }
            return 1;
        } else if (peek_next(lexer) == '*') {
            // Multi-line comment
            advance(lexer);
            advance(lexer);
            while (lexer->source[lexer->pos] != '\0') {
                if (lexer->source[lexer->pos] == '*' && lexer->source[lexer->pos + 1] == '/') {
                    advance(lexer);
                    advance(lexer);
                    return 1;
                }
                advance(lexer);
            }
            lexer_error(lexer, "Unterminated multi-line comment");
            return 0;
        }
    }
    return 0;
}

static char* read_string(Lexer *lexer) {
    char quote = peek(lexer);
    advance(lexer);
    
    int start_pos = lexer->pos;
    int capacity = 256;
    char *result = (char*)malloc(capacity);
    int length = 0;
    
    while (lexer->source[lexer->pos] != '\0' && lexer->source[lexer->pos] != quote) {
        if (lexer->source[lexer->pos] == '\\') {
            advance(lexer);
            if (lexer->source[lexer->pos] != '\0') {
                if (length >= capacity - 1) {
                    capacity *= 2;
                    result = (char*)realloc(result, capacity);
                }
                
                switch (lexer->source[lexer->pos]) {
                    case 'n': result[length++] = '\n'; break;
                    case 't': result[length++] = '\t'; break;
                    case '\\': result[length++] = '\\'; break;
                    case '"': result[length++] = '"'; break;
                    case '\'': result[length++] = '\''; break;
                    default: result[length++] = lexer->source[lexer->pos]; break;
                }
                advance(lexer);
            }
        } else {
            if (length >= capacity - 1) {
                capacity *= 2;
                result = (char*)realloc(result, capacity);
            }
            result[length++] = lexer->source[lexer->pos];
            advance(lexer);
        }
    }
    
    result[length] = '\0';
    
    if (lexer->source[lexer->pos] == '\0') {
        free(result);
        lexer_error(lexer, "Unterminated string literal");
        return NULL;
    }
    
    advance(lexer); // Skip closing quote
    return result;
}

static char* read_number(Lexer *lexer) {
    int start_pos = lexer->pos;
    int is_float = 0;
    
    while (lexer->source[lexer->pos] != '\0' && 
           (isdigit(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '.')) {
        if (lexer->source[lexer->pos] == '.') {
            if (is_float) break;
            is_float = 1;
        }
        advance(lexer);
    }
    
    int length = lexer->pos - start_pos;
    char *number = (char*)malloc(length + 1);
    strncpy(number, &lexer->source[start_pos], length);
    number[length] = '\0';
    
    return number;
}

static char* read_identifier(Lexer *lexer) {
    int start_pos = lexer->pos;
    
    while (lexer->source[lexer->pos] != '\0' && 
           (isalnum(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_')) {
        advance(lexer);
    }
    
    int length = lexer->pos - start_pos;
    char *identifier = (char*)malloc(length + 1);
    strncpy(identifier, &lexer->source[start_pos], length);
    identifier[length] = '\0';
    
    return identifier;
}

static TokenType lookup_keyword(const char *identifier) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(identifier, KEYWORDS[i]) == 0) {
            return KEYWORD_TYPES[i];
        }
    }
    return TOKEN_IDENTIFIER;
}

static int is_operator_char(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
           c == '=' || c == '!' || c == '<' || c == '>' || c == ':' ||
           c == '?' || c == '.';
}

int tokenize(const char *source, Token **tokens, int *token_count) {
    Lexer lexer;
    lexer_init(&lexer, source);
    
    while (lexer.source[lexer.pos] != '\0') {
        skip_whitespace(&lexer);
        
        if (lexer.source[lexer.pos] == '\0') break;
        
        if (skip_comment(&lexer)) continue;
        
        int start_line = lexer.line;
        int start_col = lexer.column;
        char current = peek(&lexer);
        
        // String literals
        if (current == '"' || current == '\'') {
            char *str_value = read_string(&lexer);
            if (!str_value) {
                lexer_free(&lexer);
                return 1;
            }
            add_token(&lexer, TOKEN_STRING, str_value, start_line, start_col);
            continue;
        }
        
        // Numbers
        if (isdigit(current)) {
            char *num_value = read_number(&lexer);
            add_token(&lexer, TOKEN_INTEGER, num_value, start_line, start_col);
            continue;
        }
        
        // Identifiers and keywords
        if (isalpha(current) || current == '_') {
            char *identifier = read_identifier(&lexer);
            TokenType type = lookup_keyword(identifier);
            add_token(&lexer, type, identifier, start_line, start_col);
            continue;
        }
        
        // Operators and delimiters
        if (current == '+' ) {
            add_token(&lexer, TOKEN_PLUS, "+", start_line, start_col);
            advance(&lexer);
        } else if (current == '-') {
            if (peek_next(&lexer) == '>') {
                add_token(&lexer, TOKEN_ARROW, "->", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_MINUS, "-", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == '*') {
            add_token(&lexer, TOKEN_MULTIPLY, "*", start_line, start_col);
            advance(&lexer);
        } else if (current == '/') {
            add_token(&lexer, TOKEN_DIVIDE, "/", start_line, start_col);
            advance(&lexer);
        } else if (current == '%') {
            add_token(&lexer, TOKEN_MODULO, "%", start_line, start_col);
            advance(&lexer);
        } else if (current == '=') {
            if (peek_next(&lexer) == '=') {
                add_token(&lexer, TOKEN_EQUAL, "==", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_ASSIGN, "=", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == '!') {
            if (peek_next(&lexer) == '=') {
                add_token(&lexer, TOKEN_NOT_EQUAL, "!=", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_QUESTION, "!", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == '<') {
            if (peek_next(&lexer) == '=') {
                add_token(&lexer, TOKEN_LESS_EQUAL, "<=", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_LESS, "<", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == '>') {
            if (peek_next(&lexer) == '=') {
                add_token(&lexer, TOKEN_GREATER_EQUAL, ">=", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_GREATER, ">", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == ':') {
            if (peek_next(&lexer) == '=') {
                add_token(&lexer, TOKEN_COLON_ASSIGN, ":=", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_COLON, ":", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == '?') {
            if (peek_next(&lexer) == '.') {
                add_token(&lexer, TOKEN_QUESTION_DOT, "?.", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else if (peek_next(&lexer) == '?') {
                add_token(&lexer, TOKEN_NULL_COALESCE, "??", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_QUESTION, "?", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == '.') {
            if (peek_next(&lexer) == '.') {
                add_token(&lexer, TOKEN_RANGE, "..", start_line, start_col);
                advance(&lexer);
                advance(&lexer);
            } else {
                add_token(&lexer, TOKEN_DOT, ".", start_line, start_col);
                advance(&lexer);
            }
        } else if (current == ';') {
            add_token(&lexer, TOKEN_SEMICOLON, ";", start_line, start_col);
            advance(&lexer);
        } else if (current == ',') {
            add_token(&lexer, TOKEN_COMMA, ",", start_line, start_col);
            advance(&lexer);
        } else if (current == '(') {
            add_token(&lexer, TOKEN_LEFT_PAREN, "(", start_line, start_col);
            advance(&lexer);
        } else if (current == ')') {
            add_token(&lexer, TOKEN_RIGHT_PAREN, ")", start_line, start_col);
            advance(&lexer);
        } else if (current == '{') {
            add_token(&lexer, TOKEN_LEFT_BRACE, "{", start_line, start_col);
            advance(&lexer);
        } else if (current == '}') {
            add_token(&lexer, TOKEN_RIGHT_BRACE, "}", start_line, start_col);
            advance(&lexer);
        } else if (current == '[') {
            add_token(&lexer, TOKEN_LEFT_BRACKET, "[", start_line, start_col);
            advance(&lexer);
        } else if (current == ']') {
            add_token(&lexer, TOKEN_RIGHT_BRACKET, "]", start_line, start_col);
            advance(&lexer);
        } else {
            char error_msg[100];
            snprintf(error_msg, sizeof(error_msg), "Unexpected character: %c", current);
            lexer_error(&lexer, error_msg);
            lexer_free(&lexer);
            return 1;
        }
    }
    
    // Add EOF token
    add_token(&lexer, TOKEN_EOF, NULL, lexer.line, lexer.column);
    
    *tokens = lexer.tokens;
    *token_count = lexer.token_count;
    
    return 0;
}

void free_tokens(Token *tokens, int count) {
    if (tokens) {
        for (int i = 0; i < count; i++) {
            if (tokens[i].value) {
                free(tokens[i].value);
            }
        }
        free(tokens);
    }
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_FUNC: return "FUNC";
        case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_INT: return "INT";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_COLON_ASSIGN: return "COLON_ASSIGN";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}

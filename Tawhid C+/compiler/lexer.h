/*
 * Tawhid C+ Lexer Header
 */

#ifndef LEXER_H
#define LEXER_H

typedef enum {
    // Literals
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHARACTER,
    
    // Keywords
    TOKEN_FUNC,
    TOKEN_IMPORT,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_INT,
    TOKEN_FLOAT_TYPE,
    TOKEN_DOUBLE,
    TOKEN_CHAR,
    TOKEN_VOID,
    TOKEN_BOOL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_STRING_TYPE,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_DEFER,
    TOKEN_MATCH,
    TOKEN_UNSAFE,
    TOKEN_MODULE,
    TOKEN_EXPORT,
    TOKEN_IN,
    
    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_ASSIGN,
    TOKEN_COLON_ASSIGN,  // :=
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_ARROW,         // ->
    TOKEN_QUESTION,      // ?
    TOKEN_NULL_COALESCE, // ??
    
    // Delimiters
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_QUESTION_DOT,  // ?.
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_RANGE,         // ..
    
    // End of file
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

/* Function declarations */
int tokenize(const char *source, Token **tokens, int *token_count);
void free_tokens(Token *tokens, int count);
const char *token_type_to_string(TokenType type);

#endif /* LEXER_H */

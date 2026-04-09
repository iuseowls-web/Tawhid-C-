/*
 * Tawhid C+ Parser Header
 */

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_VARIABLE_DECL,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_BLOCK,
    AST_RETURN,
    AST_IF,
    AST_FOR,
    AST_WHILE,
    AST_CALL,
    AST_STRUCT,
    AST_IMPORT,
    AST_PARAMETER,
    AST_MEMBER_ACCESS,
    AST_ARRAY_ACCESS,
    AST_TRY_CATCH,
    AST_DEFER,
    AST_MATCH
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *value;
    struct ASTNode **children;
    int child_count;
    int line;
    int column;
} ASTNode;

/* Function declarations */
int parse(Token *tokens, int token_count, ASTNode **ast);
void free_ast(ASTNode *node);

#endif /* PARSER_H */

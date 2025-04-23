#ifndef TRANSPILER_H
#define TRANSPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TOKEN_NUMBER 1
#define TOKEN_OPERATOR 2
#define TOKEN_PARENTHESIS 3

typedef struct
{
    int type;
    int value;
} Token;

struct AST_Node
{
    Token token;
    struct AST_Node *left;
    struct AST_Node *right;

};

typedef struct AST_Node AST_Node;

int parse(Token **tokens);

AST_Node *factor(int len, Token *tokens, int *pos);
AST_Node *expression(int len, Token *tokens, int *pos);
AST_Node *term(int len, Token *tokens, int *pos);

AST_Node *create_node(Token token);
void tree_destroy(AST_Node *node);

void print_ast(AST_Node *node);
void print_subtree(AST_Node *node, const char *prefix, int is_left);

void asm_write(AST_Node *ast);
int asm_gen(FILE *output_file, AST_Node *node);

#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TOKEN_NUMBER 1
#define TOKEN_OPERATOR 2
#define TOKEN_PARENTHESIS 3

typedef struct {
    int type;
    int value;
} Token;

typedef struct AST_Node AST_Node;

struct AST_Node{
    Token token;
    AST_Node *left;
    AST_Node *right;
};

int pos = 0;

AST_Node *factor(Token *tokens, int len);
AST_Node *expression(Token *tokens, int len);
AST_Node *term(Token *tokens, int len);

int parse(Token **tokens) {
    int lookup, acc = 0, digit_flag = 0, i = 0;

    Token *p = malloc(sizeof(Token) * 1000);
    if (!p) {
        perror("Allocation error in parse function: ");
        exit(EXIT_FAILURE);
    }
    *tokens = p;
    do
    {
        lookup = getchar();
        if (isdigit(lookup)) {
            digit_flag = 1;
            acc = acc*10 + lookup - '0';
        }
        else {
            if (digit_flag == 1) {
                p[i].type = TOKEN_NUMBER;
                p[i].value = acc;
                acc = digit_flag = 0;
                i++;
            }
            acc = digit_flag = 0;
            if (strchr("+-*/", lookup)) {
                p[i].type = TOKEN_OPERATOR;
                p[i].value = lookup;
                i++;
            }
            else if (lookup == '(') {
                p[i].type = TOKEN_PARENTHESIS;
                p[i].value = lookup;
                i++;
            }
            else if (lookup == ')') {
                p[i].type = TOKEN_PARENTHESIS;
                p[i].value = lookup;
                i++;
            }
            else if (lookup == EOF || isspace(lookup));
            else {
            fprintf(stderr, "Error: Unknown character '%c'!\n", lookup);
            exit(EXIT_FAILURE);
            }
        }
    } while (lookup!=EOF && lookup!='\n');

    Token *temp = realloc(p, sizeof(Token) * i);
    if (!temp) {
        perror("Allocation error in parse function: ");
        exit(EXIT_FAILURE);
    }
    *tokens = temp;

    return i;
}

AST_Node *create_node(Token token) {

    AST_Node *node = malloc(sizeof(AST_Node));

    if (!node) {
        perror("Allocation error in create_node function");
        exit(EXIT_FAILURE);
    }

    node->token = token;
    node->right = NULL;
    node->left = NULL;
    return node;
}

// F -> N | (E)
AST_Node *factor(Token *tokens, int len) {
    Token next = tokens[pos];
    AST_Node *result;
    if (next.type == TOKEN_PARENTHESIS && next.value == '(') {
        next = tokens[++pos];
        if (next.type == TOKEN_OPERATOR && next.value == '-') {
            next = tokens[++pos];
            next.value = next.value * (-1);

            pos+=2;

            AST_Node *node = create_node(next);
            return node;
        }

        result = expression(tokens, len);
        Token closingBracket;
        if (pos < len) {
            closingBracket = tokens[pos];
        } else {
            fprintf(stderr, "Unexpected end of expression!\n");
            exit(EXIT_FAILURE);
        }
        if (closingBracket.type == TOKEN_PARENTHESIS && closingBracket.value == ')') {
            pos++;
            return result;
        } else {
            fprintf(stderr, "Error: Unexpected character '%c'!\n", closingBracket);
            exit(EXIT_FAILURE);
        }
        
    }
    pos++;

    AST_Node *node = create_node(next);
    return node;
}

//T -> F*/F*/F*/*/ ... */F
AST_Node *term(Token *tokens, int len) {

    AST_Node *first = factor(tokens, len);

    while (pos < len) {
        Token operator = tokens[pos];
        if (operator.type == TOKEN_OPERATOR && (operator.value == '*' || operator.value == '/')) {
            pos++;

        if (tokens[pos].type == TOKEN_OPERATOR) {
            fprintf(stderr, "Error: Unexpected character '%c'!\n", tokens[pos].value);
            exit(EXIT_FAILURE);
        }

        AST_Node *second = factor(tokens, len);
        
        AST_Node *node = create_node(operator);

        node->right = first;
        node->left = second;

        first = node;
        }
        else {
            break;
        }
    }
    return first;
}

//E -> T±T±T±T± ... ±T
AST_Node *expression(Token *tokens, int len) {

    AST_Node *first = term(tokens, len);

    while (pos < len) {
        Token operator = tokens[pos];
        if (operator.type == TOKEN_OPERATOR && (operator.value == '+' || operator.value == '-')) {
            pos++;

        if (tokens[pos].type == TOKEN_OPERATOR) {
            fprintf(stderr, "Error: Unexpected character '%c'!\n", tokens[pos].value);
            exit(EXIT_FAILURE);
        }

        AST_Node *second = term(tokens, len);
        
        AST_Node *node = create_node(operator);
        node->right = first;
        node->left = second;
        
        first = node;
        }
        else {
            break;
        }
        
    }
    return first;
}

void print_subtree(AST_Node* node, const char* prefix, int is_left) {
    if (node == NULL) return;
    
    printf("%s", prefix);
    printf(is_left ? "└──" : "├──");
    
    if (node->token.type == TOKEN_OPERATOR) printf(" %c\n", node->token.value);
    else printf(" %d\n", node->token.value);

    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_left ? "    " : "│   ");

    print_subtree(node->right, new_prefix, 0);
    print_subtree(node->left, new_prefix, 1);
}

void print_ast(AST_Node* root) {
    if (root == NULL) return;
    printf("%c\n", root->token.value);

    print_subtree(root->right, "", 0);
    print_subtree(root->left, "", 1);
}

void tree_destroy (AST_Node *tree) {
    if (tree==NULL) return;
    tree_destroy(tree->right);
    tree_destroy(tree->left);
    free(tree);
}

int main() {
    int len;
    Token *tokens = NULL;
    len = parse(&tokens);
    if (len == 0) {
        fprintf(stderr, "Error: Empty expression\n");
        free(tokens);
        exit(EXIT_FAILURE);
    }
    AST_Node *root = expression(tokens, len);
    print_ast(root);
    
    tree_destroy(root);
    free(tokens);
}


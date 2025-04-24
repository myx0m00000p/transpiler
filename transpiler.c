#include "transpiler.h"

int parse(Token **tokens)
{
    int lookup, acc = 0, digit_flag = 0, i = 0;

    Token *p = malloc(sizeof(Token) * 1000);
    if (!p)
    {
        perror("Allocation error in parse function");
        exit(EXIT_FAILURE);
    }

    *tokens = p;

    do
    {
        lookup = getchar();
        if (isdigit(lookup))
        {
            digit_flag = 1;
            acc = acc * 10 + lookup - '0';
        }
        else
        {
            if (digit_flag == 1)
            {
                p[i].type = TOKEN_NUMBER;
                p[i].value = acc;
                i++;
            }
            acc = digit_flag = 0;

            switch (lookup)
            {
            case '+':
            case '-':
            case '*':
            case '/':
                p[i].type = TOKEN_OPERATOR;
                p[i].value = lookup;
                i++;
                break;
            case '(':
            case ')':
                p[i].type = TOKEN_PARENTHESIS;
                p[i].value = lookup;
                i++;
                break;
            case EOF:
            case ' ':
            case '\t':
            case '\n':
                break;
            default:
                fprintf(stderr, "Error: Unknown character '%c'!\n", lookup);
                free(*tokens);
                exit(EXIT_FAILURE);
            }
        }
    } while (lookup != EOF && lookup != '\n' && i < 999);

    if (i > 0)
    {
        Token *temp = realloc(p, sizeof(Token) * i);
        if (!temp)
        {
            perror("Allocation error in parse function");
            free(*tokens);
            exit(EXIT_FAILURE);
        }
        *tokens = temp;
    }

    return i;
}

// F -> N | (E)
AST_Node *factor(int len, Token *tokens, int *pos)
{
    Token next = tokens[*pos];
    AST_Node *result;
    if (next.type == TOKEN_PARENTHESIS && next.value == '(')
    {
        next = tokens[++(*pos)];
        if (next.type == TOKEN_OPERATOR && next.value == '-')
        {
            next = tokens[++(*pos)];
            next.value = next.value * (-1);

            *pos += 2;

            AST_Node *node = create_node(next);
            return node;
        }

        result = expression(len, tokens, pos);
        Token closingBracket;
        if (*pos < len)
        {
            closingBracket = tokens[*pos];
        }
        else
        {
            fprintf(stderr, "Error: Unexpected end of expression!\n");
            free(tokens);
            exit(EXIT_FAILURE);
        }
        if (closingBracket.type == TOKEN_PARENTHESIS && closingBracket.value == ')')
        {
            (*pos)++;
            return result;
        }
        else
        {
            fprintf(stderr, "Error: Unexpected character '%c'!\n", closingBracket.value);
            free(tokens);
            exit(EXIT_FAILURE);
        }
    }
    (*pos)++;

    AST_Node *node = create_node(next);
    return node;
}

// T -> F*/F*/F*/*/ ... */F
AST_Node *term(int len, Token *tokens, int *pos)
{

    AST_Node *first = factor(len, tokens, pos);

    while (*pos < len)
    {
        Token operator= tokens[*pos];
        if (operator.type == TOKEN_OPERATOR &&(operator.value == '*' || operator.value == '/'))
        {
            (*pos)++;

            if (*pos == len)
            {
                fprintf(stderr, "Error: Unexpected end of expression!\n");
                free(tokens);
                exit(EXIT_FAILURE);
            }

            if (tokens[(*pos)].type == TOKEN_OPERATOR)
            {
                fprintf(stderr, "Error: Unexpected character '%c'!\n", tokens[*pos].value);
                free(tokens);
                exit(EXIT_FAILURE);
            }

            AST_Node *second = factor(len, tokens, pos);
            AST_Node *node = create_node(operator);

            node->right = first;
            node->left = second;

            first = node;
        }
        else
        {
            break;
        }
    }
    return first;
}

// E -> T±T±T±T± ... ±T
AST_Node *expression(int len, Token *tokens, int *pos)
{

    AST_Node *first = term(len, tokens, pos);

    while (*pos < len)
    {
        Token operator= tokens[*pos];
        if (operator.type == TOKEN_OPERATOR &&(operator.value == '+' || operator.value == '-'))
        {
            (*pos)++;

            if (*pos == len)
            {
                fprintf(stderr, "Error: Unexpected end of expression!\n");
                free(tokens);
                exit(EXIT_FAILURE);
            }

            if (tokens[*pos].type == TOKEN_OPERATOR)
            {
                fprintf(stderr, "Error: Unexpected character '%c'!\n", tokens[*pos].value);
                free(tokens);
                exit(EXIT_FAILURE);
            }

            AST_Node *second = term(len, tokens, pos);
            AST_Node *node = create_node(operator);

            node->right = first;
            node->left = second;

            first = node;
        }
        else
        {
            break;
        }
    }
    return first;
}

AST_Node *create_node(Token token)
{

    AST_Node *node = malloc(sizeof(AST_Node));

    if (!node)
    {
        perror("Allocation error in create_node function");
        exit(EXIT_FAILURE);
    }

    node->token = token;
    node->right = NULL;
    node->left = NULL;
    return node;
}

void tree_destroy(AST_Node *node)
{
    if (node == NULL)
        return;
    tree_destroy(node->left);
    tree_destroy(node->right);
    free(node);
}

void print_ast(AST_Node *node)
{
    if (node == NULL)
        return;
    printf("%c\n", node->token.value);

    print_subtree(node->right, "", 0);
    print_subtree(node->left, "", 1);
}

void print_subtree(AST_Node *node, const char *prefix, int is_left)
{
    if (node == NULL)
        return;

    printf("%s", prefix);
    printf(is_left ? "└──" : "├──");

    if (node->token.type == TOKEN_OPERATOR)
        printf(" %c\n", node->token.value);
    else
        printf(" %d\n", node->token.value);

    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_left ? "    " : "│   ");

    print_subtree(node->right, new_prefix, 0);
    print_subtree(node->left, new_prefix, 1);
}

void asm_write(AST_Node *ast)
{
    FILE *output_file = fopen("output.asm", "w");
    if (output_file == NULL)
    {
        perror("Error writing to output file");
        fclose(output_file);
        tree_destroy(ast);
        exit(EXIT_FAILURE);
    }

    fprintf(output_file, "%%include \"macros.asm\"\n");

    fprintf(output_file, "section .text\n");
    fprintf(output_file, "\tglobal _start\n");
    fprintf(output_file, "_start:\n");

    asm_gen(output_file, ast);

    fprintf(output_file, "\ttoStr\n");
    fprintf(output_file, "\tmov rsi, buf2\n");
    fprintf(output_file, "\tprint\n");

    fprintf(output_file, "\tmov rax, 60\n");
    fprintf(output_file, "\txor rdi, rdi\n");
    fprintf(output_file, "\tsyscall\n");

    fprintf(output_file, "section .bss\n");
    fprintf(output_file, "\tbuf:    resb 21\n");
    fprintf(output_file, "\tbuf2:   resb 21\n");

    fclose(output_file);
}

void asm_gen(FILE *output_file, AST_Node *node)
{
    if (node->token.type == TOKEN_NUMBER)
    {
        fprintf(output_file, "\tmov rax, %d\n", node->token.value);
    }
    else
    {
        asm_gen(output_file, node->left);
        fprintf(output_file, "\tpush rax\n");

        asm_gen(output_file, node->right);
        fprintf(output_file, "\tpop rbx\n");

        switch (node->token.value)
        {
        case '+':
            fprintf(output_file, "\tadd rax, rbx\n");
            break;
        case '-':
            fprintf(output_file, "\tsub rax, rbx\n");
            break;
        case '*':
            fprintf(output_file, "\timul rax, rbx\n");
            break;
        case '/':
            fprintf(output_file, "\tcqo\n");
            fprintf(output_file, "\tidiv rbx\n");
            break;
        default:
            fprintf(stderr, "Error: Unexpected character '%c' in the AST!\n", node->token.value);
            break;
        }
    }
}

void exec()
{
    int err;
    err = system("nasm -f elf64 output.asm -o output.o");
    if (err)
    {
        fprintf(stderr, "Error: missing output.asm file\n");
        exit(EXIT_FAILURE);
    }

    err = system("ld output.o -o output");
    if (err)
    {
        fprintf(stderr, "Error: missing output.o file\n");
        exit(EXIT_FAILURE);
    }

    err = system("./output");
    if (err)
    {
        fprintf(stderr, "Error: can't execute file\n");
        exit(EXIT_FAILURE);
    }

    printf("\n");
}

int main()
{
    int len;

    Token *tokens = NULL;

    len = parse(&tokens);
    if (len == 0)
    {
        fprintf(stderr, "Error: Empty expression\n");
        free(tokens);
        return 1;
    }

    int pos = 0;
    AST_Node *ast = NULL;
    ast = expression(len, tokens, &pos);

    free(tokens);

    print_ast(ast);

    asm_write(ast);

    tree_destroy(ast);

    exec();

    return 0;
}

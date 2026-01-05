#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"

ASTNode* create_node(NodeType type, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->data_type = TYPE_VOID;
    node->line = line;
    node->left = NULL; node->right = NULL; 
    node->else_body = NULL; node->next = NULL; 
    node->id = NULL;
    return node;
}

ASTNode* create_int(int val, int line) {
    ASTNode* node = create_node(NODE_NUM_INT, line);
    node->int_val = val; node->data_type = TYPE_INT; return node;
}

ASTNode* create_float(float val, int line) {
    ASTNode* node = create_node(NODE_NUM_FLOAT, line);
    node->float_val = val; node->data_type = TYPE_FLOAT; return node;
}

ASTNode* create_var(char* name, int line) {
    ASTNode* node = create_node(NODE_VAR, line);
    node->id = strdup(name); return node;
}

ASTNode* create_decl(char* name, DataType type, int line) {
    ASTNode* node = create_node(NODE_DECL, line);
    node->id = strdup(name); node->data_type = type; return node;
}

ASTNode* create_binop(char* op, ASTNode* left, ASTNode* right, int line) {
    ASTNode* node = create_node(NODE_BINOP, line);
    node->id = strdup(op); node->left = left; node->right = right; return node;
}

ASTNode* create_assign(char* var_name, ASTNode* expr, int line) {
    ASTNode* node = create_node(NODE_ASSIGN, line);
    node->id = strdup(var_name); node->left = expr; return node;
}

ASTNode* create_if(ASTNode* cond, ASTNode* body, ASTNode* else_body, int line) {
    ASTNode* node = create_node(NODE_IF, line);
    node->left = cond; node->right = body; node->else_body = else_body; return node;
}

ASTNode* create_unless(ASTNode* cond, ASTNode* body, int line) {
    ASTNode* node = create_node(NODE_UNLESS, line);
    node->left = cond; node->right = body; return node;
}

ASTNode* create_while(ASTNode* cond, ASTNode* body, int line) {
    ASTNode* node = create_node(NODE_WHILE, line);
    node->left = cond; node->right = body; return node;
}

ASTNode* create_read(char* var_name, int line) {
    ASTNode* node = create_node(NODE_READ, line);
    node->id = strdup(var_name); return node;
}

ASTNode* create_print(ASTNode* expr, int line) {
    ASTNode* node = create_node(NODE_PRINT, line);
    node->left = expr; return node;
}

ASTNode* create_block(ASTNode* statements, int line) {
    ASTNode* node = create_node(NODE_BLOCK, line);
    node->left = statements; return node;
}

// --- YENI FONKSIYONLAR ---
ASTNode* create_func_decl(char* name, DataType ret_type, ASTNode* params, ASTNode* body, int line) {
    ASTNode* node = create_node(NODE_FUNC_DECL, line);
    node->id = strdup(name);
    node->data_type = ret_type;
    node->left = params;  // Parametre listesi
    node->right = body;   // Fonksiyon gövdesi
    return node;
}

ASTNode* create_func_call(char* name, ASTNode* args, int line) {
    ASTNode* node = create_node(NODE_FUNC_CALL, line);
    node->id = strdup(name);
    node->left = args;    // Argüman listesi
    return node;
}

ASTNode* create_return(ASTNode* expr, int line) {
    ASTNode* node = create_node(NODE_RETURN, line);
    node->left = expr;
    return node;
}

ASTNode* create_param(char* name, DataType type, int line) {
    ASTNode* node = create_node(NODE_PARAM, line);
    node->id = strdup(name);
    node->data_type = type;
    return node;
}

void print_ast_tree(ASTNode* node, int depth) {
    if (node == NULL) return;
    for (int i = 0; i < depth; i++) printf("  | ");

    switch (node->type) {
        case NODE_PROGRAM: printf("PROGRAM\n"); break;
        case NODE_FUNC_DECL: printf("FUNCTION: %s (Ret: %s)\n", node->id, (node->data_type==TYPE_INT?"INT":"FLOAT")); break;
        case NODE_PARAM:   printf("PARAM: %s\n", node->id); break;
        case NODE_FUNC_CALL: printf("CALL: %s\n", node->id); break;
        case NODE_RETURN:  printf("RETURN\n"); break;
        case NODE_BLOCK:   printf("BLOCK\n"); break;
        case NODE_DECL:    printf("DECL: %s\n", node->id); break;
        case NODE_ASSIGN:  printf("ASSIGN: %s\n", node->id); break;
        case NODE_IF:      printf("IF\n"); break;
        case NODE_WHILE:   printf("WHILE\n"); break;
        case NODE_READ:    printf("READ: %s\n", node->id); break;
        case NODE_PRINT:   printf("PRINT\n"); break;
        case NODE_BINOP:   printf("OP: %s\n", node->id); break;
        case NODE_NUM_INT: printf("NUM_INT: %d\n", node->int_val); break;
        case NODE_NUM_FLOAT: printf("NUM_FLOAT: %f\n", node->float_val); break;
        case NODE_VAR:     printf("VAR: %s\n", node->id); break;
        default:           printf("UNKNOWN\n"); break;
    }

    print_ast_tree(node->left, depth + 1);
    if (node->else_body) {
        for (int i = 0; i < depth; i++) printf("  | ELSE\n");
        print_ast_tree(node->else_body, depth + 1);
    }
    print_ast_tree(node->right, depth + 1);
    print_ast_tree(node->next, depth);
}

void free_ast(ASTNode* node) {
    if (node == NULL) return;
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->else_body);
    free_ast(node->next);
    if (node->id != NULL) free(node->id);
    free(node);
}
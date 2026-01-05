#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAM, NODE_BLOCK, NODE_DECL, NODE_ASSIGN,
    NODE_IF, NODE_UNLESS, NODE_WHILE,
    NODE_READ, NODE_PRINT, 
    NODE_BINOP, NODE_VAR, NODE_NUM_INT, NODE_NUM_FLOAT,
    // --- YENI EKLENENLER ---
    NODE_FUNC_DECL, // Fonksiyon Tanimlama
    NODE_FUNC_CALL, // Fonksiyon Cagirma
    NODE_RETURN,    // Return ifadesi
    NODE_PARAM      // Parametre
} NodeType;

typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT
} DataType;

typedef struct ASTNode {
    NodeType type;
    DataType data_type; 
    char* id;
    int int_val;
    float float_val;
    int line;
    struct ASTNode *left;      // Sol cocuk (Genelde ifade veya parametre listesi)
    struct ASTNode *right;     // Sag cocuk (Genelde Govde / Body)
    struct ASTNode *else_body; // If-Else icin
    struct ASTNode *next;      // Bagli liste (Statement listesi veya argumanlar)
} ASTNode;

// Node Olusturma Fonksiyonlari
ASTNode* create_node(NodeType type, int line);
ASTNode* create_int(int val, int line);
ASTNode* create_float(float val, int line);
ASTNode* create_var(char* name, int line);
ASTNode* create_decl(char* name, DataType type, int line);
ASTNode* create_binop(char* op, ASTNode* left, ASTNode* right, int line);
ASTNode* create_assign(char* var_name, ASTNode* expr, int line);
ASTNode* create_if(ASTNode* cond, ASTNode* body, ASTNode* else_body, int line);
ASTNode* create_unless(ASTNode* cond, ASTNode* body, int line);
ASTNode* create_while(ASTNode* cond, ASTNode* body, int line);
ASTNode* create_read(char* var_name, int line);
ASTNode* create_print(ASTNode* expr, int line);
ASTNode* create_block(ASTNode* statements, int line);

// --- YENI FONKSIYONLAR ---
ASTNode* create_func_decl(char* name, DataType ret_type, ASTNode* params, ASTNode* body, int line);
ASTNode* create_func_call(char* name, ASTNode* args, int line);
ASTNode* create_return(ASTNode* expr, int line);
ASTNode* create_param(char* name, DataType type, int line);

void print_ast_tree(ASTNode* node, int depth);
void free_ast(ASTNode* node);

#endif
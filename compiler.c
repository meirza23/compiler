#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "compiler.h"

#define MAX_VARS 100
#define MAX_FUNCS 50
#define MAX_PARAMS 10

// --- SEMBOL TABLOSU (DEGISKENLER ICIN) ---
typedef struct {
    char name[32];
    DataType type;
    int scope_level;
    int active;
} Symbol;

Symbol symbol_table[MAX_VARS];
int symbol_count = 0;
int current_scope = 0;

// --- FONKSIYON TABLOSU ---
typedef struct {
    char name[32];
    DataType return_type;
    int param_count;
    DataType param_types[MAX_PARAMS];
} FunctionSymbol;

FunctionSymbol func_table[MAX_FUNCS];
int func_count = 0;

// --- YARDIMCI FONKSIYONLAR ---

int lookup_symbol(char* name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].active) return i; 
    }
    return -1;
}

int lookup_symbol_vm(char* name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0) return i; 
    }
    return -1;
}

int lookup_current_scope(char* name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (symbol_table[i].active && symbol_table[i].scope_level < current_scope) break;
        if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].scope_level == current_scope && symbol_table[i].active) return i;
    }
    return -1;
}

void add_symbol(char* name, DataType type, int line) {
    if (lookup_current_scope(name) != -1) {
        fprintf(stderr, "Hata (Satir %d): '%s' zaten tanimli!\n", line, name);
        exit(1);
    }
    strcpy(symbol_table[symbol_count].name, name);
    symbol_table[symbol_count].type = type;
    symbol_table[symbol_count].scope_level = current_scope;
    symbol_table[symbol_count].active = 1;
    symbol_count++;
}

void exit_scope() {
    for (int i = 0; i < symbol_count; i++) {
        if (symbol_table[i].scope_level == current_scope) symbol_table[i].active = 0;
    }
    current_scope--;
}

void add_function(char* name, DataType ret_type, int line) {
    for(int i=0; i<func_count; i++) {
        if(strcmp(func_table[i].name, name) == 0) {
            fprintf(stderr, "Hata (Satir %d): Fonksiyon '%s' zaten tanimli!\n", line, name);
            exit(1);
        }
    }
    strcpy(func_table[func_count].name, name);
    func_table[func_count].return_type = ret_type;
    func_table[func_count].param_count = 0;
    func_count++;
}

int lookup_function(char* name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(func_table[i].name, name) == 0) return i;
    }
    return -1;
}

// --- SEMANTIK ANALIZ ---

void register_functions(ASTNode* node) {
    while(node != NULL) {
        if (node->type == NODE_FUNC_DECL) {
            add_function(node->id, node->data_type, node->line);
            ASTNode* param = node->left;
            int f_idx = func_count - 1;
            while(param != NULL) {
                if(func_table[f_idx].param_count < MAX_PARAMS) {
                    func_table[f_idx].param_types[func_table[f_idx].param_count++] = param->data_type;
                }
                param = param->next;
            }
        }
        node = node->next;
    }
}

int analyze_node(ASTNode* node) {
    if (node == NULL) return 0;

    switch (node->type) {
        case NODE_PROGRAM: 
            register_functions(node->left); 
            return analyze_node(node->left);

        case NODE_FUNC_DECL:
            current_scope++; 
            ASTNode* param = node->left;
            while(param != NULL) {
                add_symbol(param->id, param->data_type, param->line);
                param = param->next;
            }
            if (analyze_node(node->right) != 0) return 1;
            exit_scope();
            break;

        case NODE_BLOCK:
            current_scope++;
            if (analyze_node(node->left) != 0) return 1;
            exit_scope();
            break;

        case NODE_DECL:
            add_symbol(node->id, node->data_type, node->line);
            break;

        case NODE_ASSIGN:
            if (lookup_symbol(node->id) == -1) {
                fprintf(stderr, "Hata (Satir %d): Tanimlanmamis degisken '%s'!\n", node->line, node->id);
                return 1;
            }
            if (analyze_node(node->left) != 0) return 1;
            int idx = lookup_symbol(node->id);
            if (symbol_table[idx].type != node->left->data_type) {
                fprintf(stderr, "HATA (Satir %d): Tip uyusmazligi! Degisken %s.\n", node->line, node->id);
                return 1;
            }
            break;

        case NODE_VAR:
        case NODE_READ:
            if (lookup_symbol(node->id) == -1) {
                fprintf(stderr, "Hata (Satir %d): Tanimlanmamis degisken '%s'!\n", node->line, node->id);
                return 1;
            }
            node->data_type = symbol_table[lookup_symbol(node->id)].type;
            break;

        case NODE_FUNC_CALL:
            {
                int f_idx = lookup_function(node->id);
                if (f_idx == -1) {
                    fprintf(stderr, "Hata (Satir %d): Tanimlanmamis fonksiyon '%s'!\n", node->line, node->id);
                    return 1;
                }
                node->data_type = func_table[f_idx].return_type;
                
                ASTNode* arg = node->left;
                int arg_count = 0;
                while(arg != NULL) {
                    analyze_node(arg); 
                    if (arg_count < func_table[f_idx].param_count) {
                        if (arg->data_type != func_table[f_idx].param_types[arg_count]) {
                            fprintf(stderr, "Hata (Satir %d): '%s' icin %d. arguman tipi hatali!\n", node->line, node->id, arg_count+1);
                            return 1;
                        }
                    }
                    arg_count++;
                    arg = arg->next;
                }
                if (arg_count != func_table[f_idx].param_count) {
                    fprintf(stderr, "Hata (Satir %d): '%s' %d arguman bekliyor, %d verildi.\n", node->line, node->id, func_table[f_idx].param_count, arg_count);
                    return 1;
                }
            }
            break;

        case NODE_RETURN:
            if (node->left) analyze_node(node->left);
            break;

        case NODE_IF:
        case NODE_UNLESS:
        case NODE_WHILE:
            if (analyze_node(node->left) != 0) return 1;
            if (analyze_node(node->right) != 0) return 1;
            if (node->else_body) analyze_node(node->else_body);
            break;

        case NODE_BINOP:
            if (analyze_node(node->left) != 0) return 1;
            if (analyze_node(node->right) != 0) return 1;
            if (node->left->data_type != node->right->data_type) {
                fprintf(stderr, "HATA (Satir %d): Farkli tiplerle islem yapilamaz!\n", node->line);
                return 1;
            }
            node->data_type = node->left->data_type; 
            break;

        case NODE_PRINT:
            analyze_node(node->left);
            break;
            
        case NODE_NUM_INT: node->data_type = TYPE_INT; break;
        case NODE_NUM_FLOAT: node->data_type = TYPE_FLOAT; break;
        
        case NODE_PARAM: 
            break;
    }
    return analyze_node(node->next);
}

int semantic_analysis(ASTNode* node) {
    symbol_count = 0;
    current_scope = 0;
    func_count = 0;
    return analyze_node(node);
}

// --- SANAL MAKINE (VM) & KOD URETIMI ---

typedef struct {
    DataType type;
    union { int i_val; float f_val; } val;
} StackItem;

StackItem stack[100];
int sp = -1;
StackItem memory[100]; 
int label_counter = 0;

// YENI: Dosya pointer'ı
static FILE *vm_out = NULL;

void push_int(int v) { sp++; stack[sp].type = TYPE_INT; stack[sp].val.i_val = v; }
void push_float(float v) { sp++; stack[sp].type = TYPE_FLOAT; stack[sp].val.f_val = v; }
StackItem pop() { return stack[sp--]; }

void generate_node_code(ASTNode* node);

void generate_code(ASTNode* node, const char* filename) {
    // Dosyayı yazma modunda aç
    vm_out = fopen(filename, "w");
    if (!vm_out) {
        fprintf(stderr, "Hata: Cikti dosyasi '%s' olusturulamadi!\n", filename);
        return;
    }

    ASTNode* curr = node->left; 
    while(curr != NULL) {
        if (curr->type == NODE_FUNC_DECL && strcmp(curr->id, "main") == 0) {
            fprintf(vm_out, "--- MAIN BASLIYOR ---\n");
            generate_node_code(curr->right); 
            fprintf(vm_out, "--- MAIN BITTI ---\n");
            fclose(vm_out); // İşi bitince kapat
            return;
        }
        curr = curr->next;
    }
    printf("UYARI: 'main' fonksiyonu bulunamadi, kod uretilmedi.\n");
    fclose(vm_out);
}

void generate_node_code(ASTNode* node) {
    if (node == NULL) return;
    int lbl1, lbl2;

    switch (node->type) {
        case NODE_PROGRAM: 
            break;

        case NODE_BLOCK: generate_node_code(node->left); break;
        case NODE_DECL: fprintf(vm_out, "DECLARE %s\n", node->id); break;
        
        case NODE_ASSIGN:
            generate_node_code(node->left);
            fprintf(vm_out, "STORE %s\n", node->id);
            int addr = lookup_symbol_vm(node->id); 
            if(addr != -1) memory[addr] = pop(); else pop();
            break;
            
        case NODE_VAR:
            fprintf(vm_out, "LOAD %s\n", node->id);
            int v_addr = lookup_symbol_vm(node->id); 
            if(v_addr != -1) { sp++; stack[sp] = memory[v_addr]; } else push_int(0);
            break;
            
        case NODE_NUM_INT: fprintf(vm_out, "PUSH_INT %d\n", node->int_val); push_int(node->int_val); break;
        case NODE_NUM_FLOAT: fprintf(vm_out, "PUSH_FLOAT %f\n", node->float_val); push_float(node->float_val); break;
        
        case NODE_PRINT:
            generate_node_code(node->left);
            StackItem res = pop();
            fprintf(vm_out, "PRINT >> "); // Bu komut dosyaya
            // Asagidaki printf'ler dosya icinde sabit deger gosteremeyecegi icin
            // burada sadece "PRINT" komutunu bytecode'a yaziyoruz. 
            // Ancak bu VM ayni zamanda "calisan" bir VM oldugu icin (simulasyon)
            // calisma sonucunu hala terminale basmasi mantikli olabilir.
            // Ama siz bytecode istediniz. Bytecode dosyasinda sonuc gorulmez, komut gorulur.
            // O yuzden dosyaya sunu yazalim:
            if (res.type == TYPE_INT) fprintf(vm_out, "[Deger: %d]\n", res.val.i_val); 
            else fprintf(vm_out, "[Deger: %f]\n", res.val.f_val);
            break;

        case NODE_WHILE:
            lbl1 = label_counter++; lbl2 = label_counter++;
            fprintf(vm_out, "LABEL_START_%d:\n", lbl1);
            while(1) { 
                generate_node_code(node->left); 
                StackItem cond = pop();
                fprintf(vm_out, "JZ LABEL_END_%d\n", lbl2);
                if ((cond.type==TYPE_INT && !cond.val.i_val) || (cond.type==TYPE_FLOAT && !cond.val.f_val)) break;
                generate_node_code(node->right); 
                fprintf(vm_out, "JMP LABEL_START_%d\n", lbl1);
            }
            fprintf(vm_out, "LABEL_END_%d:\n", lbl2);
            break;

        case NODE_IF:
            lbl1 = label_counter++; lbl2 = label_counter++;
            generate_node_code(node->left);
            fprintf(vm_out, "JZ LABEL_ELSE_%d\n", lbl1);
            StackItem c_if = pop();
            int is_true = (c_if.type==TYPE_INT ? c_if.val.i_val : c_if.val.f_val);
            if (is_true) { generate_node_code(node->right); fprintf(vm_out, "JMP LABEL_EXIT_%d\n", lbl2); }
            fprintf(vm_out, "LABEL_ELSE_%d:\n", lbl1);
            if (!is_true && node->else_body) generate_node_code(node->else_body);
            fprintf(vm_out, "LABEL_EXIT_%d:\n", lbl2);
            break;

        case NODE_UNLESS:
            lbl1 = label_counter++;
            generate_node_code(node->left);
            fprintf(vm_out, "JNZ LABEL_SKIP_%d\n", lbl1); 
            StackItem c_unless = pop();
            int u_true = (c_unless.type==TYPE_INT ? c_unless.val.i_val : c_unless.val.f_val);
            if (!u_true) { generate_node_code(node->right); }
            fprintf(vm_out, "LABEL_SKIP_%d:\n", lbl1);
            break;

        case NODE_BINOP:
            generate_node_code(node->left);
            generate_node_code(node->right);
            StackItem b = pop(); StackItem a = pop();
            if (a.type == TYPE_INT) {
                if (strcmp(node->id, "+") == 0) { fprintf(vm_out, "ADD_I\n"); push_int(a.val.i_val + b.val.i_val); }
                else if (strcmp(node->id, "-") == 0) { fprintf(vm_out, "SUB_I\n"); push_int(a.val.i_val - b.val.i_val); }
                else if (strcmp(node->id, "*") == 0) { fprintf(vm_out, "MUL_I\n"); push_int(a.val.i_val * b.val.i_val); }
                else if (strcmp(node->id, "/") == 0) { fprintf(vm_out, "DIV_I\n"); push_int(a.val.i_val / b.val.i_val); }
                else if (strcmp(node->id, "%") == 0) { fprintf(vm_out, "MOD_I\n"); push_int(a.val.i_val % b.val.i_val); }
                else if (strcmp(node->id, "^") == 0) { fprintf(vm_out, "POW_I\n"); push_int((int)pow(a.val.i_val, b.val.i_val)); }
                else if (strcmp(node->id, ">") == 0) { fprintf(vm_out, "GT_I\n"); push_int(a.val.i_val > b.val.i_val); }
                else if (strcmp(node->id, "<") == 0) { fprintf(vm_out, "LT_I\n"); push_int(a.val.i_val < b.val.i_val); }
                else if (strcmp(node->id, "==") == 0) { fprintf(vm_out, "EQ_I\n"); push_int(a.val.i_val == b.val.i_val); }
                else if (strcmp(node->id, "!=") == 0) { fprintf(vm_out, "NEQ_I\n"); push_int(a.val.i_val != b.val.i_val); }
            } else {
                if (strcmp(node->id, "+") == 0) { fprintf(vm_out, "ADD_F\n"); push_float(a.val.f_val + b.val.f_val); }
                else if (strcmp(node->id, "-") == 0) { fprintf(vm_out, "SUB_F\n"); push_float(a.val.f_val - b.val.f_val); }
                else if (strcmp(node->id, "*") == 0) { fprintf(vm_out, "MUL_F\n"); push_float(a.val.f_val * b.val.f_val); }
                else if (strcmp(node->id, "/") == 0) { fprintf(vm_out, "DIV_F\n"); push_float(a.val.f_val / b.val.f_val); }
                else if (strcmp(node->id, "^") == 0) { fprintf(vm_out, "POW_F\n"); push_float(powf(a.val.f_val, b.val.f_val)); }
                else if (strcmp(node->id, ">") == 0) { fprintf(vm_out, "GT_F\n"); push_int(a.val.f_val > b.val.f_val); }
                else if (strcmp(node->id, "<") == 0) { fprintf(vm_out, "LT_F\n"); push_int(a.val.f_val < b.val.f_val); }
            }
            break;

        case NODE_RETURN:
            generate_node_code(node->left);
            fprintf(vm_out, "RETURN (Stack Top)\n");
            break;

        case NODE_FUNC_CALL:
            fprintf(vm_out, "CALL %s\n", node->id);
            break;
            
        case NODE_READ:
            fprintf(vm_out, "READ %s\n", node->id);
            break;

        default:
            break;
    }
    generate_node_code(node->next);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_STACK 1000
#define MAX_MEMORY 1000
#define MAX_CODE 1000
#define MAX_LABELS 100
#define MAX_CALL_STACK 100

typedef enum {
    TYPE_INT,
    TYPE_FLOAT
} DataType;

typedef struct {
    DataType type;
    union { int i_val; float f_val; } val;
} StackItem;

typedef struct {
    char opcode[32];
    char arg[32];
    int arg_int;
    float arg_float;
} Instruction;

typedef struct {
    char name[32];
    int addr;
} Label;

Instruction code[MAX_CODE];
int code_size = 0;

Label labels[MAX_LABELS];
int label_count = 0;

StackItem stack[MAX_STACK];
int sp = -1;

StackItem memory[MAX_MEMORY]; // Basit hafıza modeli (adresleme için sembol tablosu gerekebilir ama burada basit map kullanacağız)
// VM'de değişken isimlerini adrese maplemek zor olabilir, o yüzden basit bir "isim -> değer" tablosu yapalım.
typedef struct {
    char name[32];
    StackItem val;
} Variable;
Variable vars[MAX_MEMORY];
int var_count = 0;

int call_stack[MAX_CALL_STACK];
int csp = -1;

void push(StackItem item) {
    if (sp >= MAX_STACK - 1) { printf("Stack Overflow!\n"); exit(1); }
    stack[++sp] = item;
}

StackItem pop() {
    if (sp < 0) { printf("Stack Underflow!\n"); exit(1); }
    return stack[sp--];
}

void push_call(int ret_addr) {
    if (csp >= MAX_CALL_STACK - 1) { printf("Call Stack Overflow!\n"); exit(1); }
    call_stack[++csp] = ret_addr;
}

int pop_call() {
    if (csp < 0) { printf("Call Stack Underflow!\n"); exit(1); }
    return call_stack[csp--];
}

int find_label(char* name) {
    for(int i=0; i<label_count; i++) {
        if(strcmp(labels[i].name, name) == 0) return labels[i].addr;
    }
    return -1;
}

int find_var(char* name) {
    for(int i=0; i<var_count; i++) {
        if(strcmp(vars[i].name, name) == 0) return i;
    }
    return -1;
}

int add_var(char* name) {
    int idx = find_var(name);
    if(idx != -1) return idx;
    strcpy(vars[var_count].name, name);
    vars[var_count].val.type = TYPE_INT;
    vars[var_count].val.val.i_val = 0;
    return var_count++;
}

void load_program(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) { printf("Dosya acilamadi: %s\n", filename); exit(1); }
    
    char line[128];
    while(fgets(line, sizeof(line), f)) {
        char op[32], arg[32];
        int n = sscanf(line, "%s %s", op, arg);
        
        if (n > 0) {
            // Label mı?
            if (op[strlen(op)-1] == ':') {
                op[strlen(op)-1] = '\0';
                strcpy(labels[label_count].name, op);
                labels[label_count].addr = code_size;
                label_count++;
            } else {
                strcpy(code[code_size].opcode, op);
                if (n > 1) {
                    strcpy(code[code_size].arg, arg);
                    if (strcmp(op, "PUSH_INT") == 0) code[code_size].arg_int = atoi(arg);
                    else if (strcmp(op, "PUSH_FLOAT") == 0) code[code_size].arg_float = atof(arg);
                }
                code_size++;
            }
        }
    }
    fclose(f);
}

void run() {
    int pc = 0;
    while(pc < code_size) {
        Instruction* instr = &code[pc];
        // printf("PC: %d, OP: %s, ARG: %s\n", pc, instr->opcode, instr->arg); // Debug

        if (strcmp(instr->opcode, "PUSH_INT") == 0) {
            StackItem item = {TYPE_INT, .val.i_val = instr->arg_int};
            push(item);
        } else if (strcmp(instr->opcode, "PUSH_FLOAT") == 0) {
            StackItem item = {TYPE_FLOAT, .val.f_val = instr->arg_float};
            push(item);
        } else if (strcmp(instr->opcode, "ADD") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {a.type, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val + b.val.i_val;
            else res.val.f_val = a.val.f_val + b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "SUB") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {a.type, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val - b.val.i_val;
            else res.val.f_val = a.val.f_val - b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "MUL") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {a.type, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val * b.val.i_val;
            else res.val.f_val = a.val.f_val * b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "DIV") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {a.type, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val / b.val.i_val;
            else res.val.f_val = a.val.f_val / b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "MOD") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {TYPE_INT, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val % b.val.i_val;
            else res.val.i_val = (int)a.val.f_val % (int)b.val.f_val; // Float mod? Cast to int for now
            push(res);
        } else if (strcmp(instr->opcode, "POW") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {a.type, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = (int)pow(a.val.i_val, b.val.i_val);
            else res.val.f_val = powf(a.val.f_val, b.val.f_val);
            push(res);
        } else if (strcmp(instr->opcode, "GT") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {TYPE_INT, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val > b.val.i_val;
            else res.val.i_val = a.val.f_val > b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "LT") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {TYPE_INT, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val < b.val.i_val;
            else res.val.i_val = a.val.f_val < b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "EQ") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {TYPE_INT, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val == b.val.i_val;
            else res.val.i_val = a.val.f_val == b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "NEQ") == 0) {
            StackItem b = pop(); StackItem a = pop();
            StackItem res = {TYPE_INT, .val.i_val = 0};
            if (a.type == TYPE_INT) res.val.i_val = a.val.i_val != b.val.i_val;
            else res.val.i_val = a.val.f_val != b.val.f_val;
            push(res);
        } else if (strcmp(instr->opcode, "PRINT") == 0) {
            StackItem item = pop();
            if (item.type == TYPE_INT) printf("%d\n", item.val.i_val);
            else printf("%f\n", item.val.f_val);
        } else if (strcmp(instr->opcode, "STORE") == 0) {
            int idx = add_var(instr->arg);
            vars[idx].val = pop();
        } else if (strcmp(instr->opcode, "LOAD") == 0) {
            int idx = find_var(instr->arg);
            if (idx == -1) { printf("Hata: Degisken bulunamadi %s\n", instr->arg); exit(1); }
            push(vars[idx].val);
        } else if (strcmp(instr->opcode, "DECLARE") == 0) {
            add_var(instr->arg);
        } else if (strcmp(instr->opcode, "JMP") == 0) {
            int addr = find_label(instr->arg);
            if (addr == -1) { printf("Hata: Label bulunamadi %s\n", instr->arg); exit(1); }
            pc = addr; continue;
        } else if (strcmp(instr->opcode, "JZ") == 0) {
            StackItem item = pop();
            int val = (item.type == TYPE_INT) ? item.val.i_val : (int)item.val.f_val;
            if (val == 0) {
                int addr = find_label(instr->arg);
                if (addr == -1) { printf("Hata: Label bulunamadi %s\n", instr->arg); exit(1); }
                pc = addr; continue;
            }
        } else if (strcmp(instr->opcode, "JNZ") == 0) {
            StackItem item = pop();
            int val = (item.type == TYPE_INT) ? item.val.i_val : (int)item.val.f_val;
            if (val != 0) {
                int addr = find_label(instr->arg);
                if (addr == -1) { printf("Hata: Label bulunamadi %s\n", instr->arg); exit(1); }
                pc = addr; continue;
            }
        } else if (strcmp(instr->opcode, "CALL") == 0) {
            int addr = find_label(instr->arg);
            if (addr == -1) { printf("Hata: Fonksiyon bulunamadi %s\n", instr->arg); exit(1); }
            push_call(pc + 1);
            pc = addr; continue;
        } else if (strcmp(instr->opcode, "RETURN") == 0) {
            int ret_addr = pop_call();
            pc = ret_addr; continue;
        } else if (strcmp(instr->opcode, "HALT") == 0) {
            break;
        }
        
        pc++;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) { printf("Kullanim: %s <bytecode_file>\n", argv[0]); return 1; }
    load_program(argv[1]);
    run();
    return 0;
}

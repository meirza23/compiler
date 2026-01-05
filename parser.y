%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "compiler.h"

extern int yylex();
extern int yylineno;
extern FILE *yyin;
void yyerror(const char *s);
ASTNode* root;
%}

%union {
    int intVal;
    float floatVal;
    char* strVal;
    struct ASTNode* node;
}

%token <strVal> TOKEN_ID
%token <intVal> TOKEN_NUM_INT
%token <floatVal> TOKEN_NUM_FLOAT
%token TOKEN_BEGIN TOKEN_END TOKEN_IF TOKEN_UNLESS TOKEN_ELSE TOKEN_WHILE 
%token TOKEN_READ TOKEN_PRINT TOKEN_RETURN
%token TOKEN_ASSIGN TOKEN_DOT TOKEN_KEY_INT TOKEN_KEY_FLOAT TOKEN_EQ TOKEN_NEQ

%type <node> program func_list func_decl params param_list statements statement 
%type <node> declaration assignment if_stmt unless_stmt while_stmt read_stmt print_stmt return_stmt block expr args

%left TOKEN_EQ TOKEN_NEQ
%left '>' '<'
%left '+' '-'
%left '*' '/' '%'
%right '^'

%%

program:
    func_list { root = create_node(NODE_PROGRAM, 0);
                root->left = $1; }
    ;

func_list:
    func_decl { $$ = $1; }
    | func_decl func_list { $1->next = $2; $$ = $1; }
    ;

func_decl:
    TOKEN_KEY_INT TOKEN_ID '(' params ')' block { 
        $$ = create_func_decl($2, TYPE_INT, $4, $6, yylineno);
    }
    | TOKEN_KEY_FLOAT TOKEN_ID '(' params ')' block { 
        $$ = create_func_decl($2, TYPE_FLOAT, $4, $6, yylineno);
    }
    ;

params:
    /* boş */ { $$ = NULL; }
    | param_list { $$ = $1; }
    ;

param_list:
    TOKEN_KEY_INT TOKEN_ID { $$ = create_param($2, TYPE_INT, yylineno); }
    | TOKEN_KEY_FLOAT TOKEN_ID { $$ = create_param($2, TYPE_FLOAT, yylineno); }
    | param_list ',' TOKEN_KEY_INT TOKEN_ID { 
        ASTNode* n = create_param($4, TYPE_INT, yylineno);
        ASTNode* temp = $1;
        while(temp->next != NULL) temp = temp->next;
        temp->next = n;
        $$ = $1;
    }
    | param_list ',' TOKEN_KEY_FLOAT TOKEN_ID { 
        ASTNode* n = create_param($4, TYPE_FLOAT, yylineno);
        ASTNode* temp = $1;
        while(temp->next != NULL) temp = temp->next;
        temp->next = n;
        $$ = $1;
    }
    ;

block:
    TOKEN_BEGIN statements TOKEN_END { $$ = create_block($2, yylineno); }
    ;

statements:
    statement { $$ = $1; }
    | statement statements { $1->next = $2; $$ = $1; }
    ;

statement:
    declaration | assignment | if_stmt | unless_stmt | while_stmt 
    | read_stmt | print_stmt | return_stmt | block
    | expr TOKEN_DOT { $$ = $1; } 
    ;

declaration:
    TOKEN_KEY_INT TOKEN_ID TOKEN_DOT { $$ = create_decl($2, TYPE_INT, yylineno); }
    | TOKEN_KEY_FLOAT TOKEN_ID TOKEN_DOT { $$ = create_decl($2, TYPE_FLOAT, yylineno); }
    ;

assignment:
    TOKEN_ID TOKEN_ASSIGN expr TOKEN_DOT { $$ = create_assign($1, $3, yylineno); }
    ;

if_stmt:
    TOKEN_IF '(' expr ')' block { $$ = create_if($3, $5, NULL, yylineno); }
    | TOKEN_IF '(' expr ')' block TOKEN_ELSE block { $$ = create_if($3, $5, $7, yylineno); }
    ;

unless_stmt:
    TOKEN_UNLESS '(' expr ')' block { $$ = create_unless($3, $5, yylineno); }
    ;

while_stmt:
    TOKEN_WHILE '(' expr ')' block { $$ = create_while($3, $5, yylineno); }
    ;

read_stmt:
    TOKEN_READ '(' TOKEN_ID ')' TOKEN_DOT { $$ = create_read($3, yylineno); }
    ;

print_stmt:
    TOKEN_PRINT '(' expr ')' TOKEN_DOT { $$ = create_print($3, yylineno); }
    ;

return_stmt:
    TOKEN_RETURN expr TOKEN_DOT { $$ = create_return($2, yylineno); }
    ;

expr:
    TOKEN_NUM_INT { $$ = create_int($1, yylineno); }
    | TOKEN_NUM_FLOAT { $$ = create_float($1, yylineno); }
    | TOKEN_ID { $$ = create_var($1, yylineno); }
    | TOKEN_ID '(' args ')' { $$ = create_func_call($1, $3, yylineno); }
    | expr '+' expr { $$ = create_binop("+", $1, $3, yylineno); }
    | expr '-' expr { $$ = create_binop("-", $1, $3, yylineno); }
    | expr '*' expr { $$ = create_binop("*", $1, $3, yylineno); }
    | expr '/' expr { $$ = create_binop("/", $1, $3, yylineno); }
    | expr '%' expr { $$ = create_binop("%", $1, $3, yylineno); }
    | expr '^' expr { $$ = create_binop("^", $1, $3, yylineno); }
    | expr '>' expr { $$ = create_binop(">", $1, $3, yylineno); }
    | expr '<' expr { $$ = create_binop("<", $1, $3, yylineno); }
    | expr TOKEN_EQ expr { $$ = create_binop("==", $1, $3, yylineno); }
    | expr TOKEN_NEQ expr { $$ = create_binop("!=", $1, $3, yylineno); }
    ;

args:
    /* boş */ { $$ = NULL; }
    | expr { $$ = $1; }
    | expr ',' args { $1->next = $3; $$ = $1; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Hata (Satir %d): %s\n", yylineno, s);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) { fprintf(stderr, "Dosya acilamadi: %s\n", argv[1]); return 1; }
        yyin = file;
    }

    if (yyparse() == 0) {
        printf("\n--- ABSTRACT SYNTAX TREE ---\n");
        print_ast_tree(root, 0);
        
        printf("\n--- SEMANTIK ANALIZ ---\n");
        if (semantic_analysis(root) == 0) {
            printf("Semantik Analiz Basarili!\n");
            
            // BURAYI DEGISTIRDIK: Çıktı dosyası ismi verildi
            const char* output_filename = "output.vm";
            printf("\n--- CODE GENERATION ---\n");
            printf("Kodlar '%s' dosyasina yaziliyor...\n", output_filename);
            
            generate_code(root, output_filename);
            
            printf("Islem tamamlandi. '%s' dosyasini kontrol edin.\n", output_filename);
        }
        free_ast(root);
    }
    return 0;
}
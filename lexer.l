%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
%}

%option noyywrap
%option yylineno

%%

"int"       { return TOKEN_KEY_INT; }
"float"     { return TOKEN_KEY_FLOAT; }
"begin"     { return TOKEN_BEGIN; }
"end"       { return TOKEN_END; }
"if"        { return TOKEN_IF; }
"unless"    { return TOKEN_UNLESS; }
"else"      { return TOKEN_ELSE; }
"while"     { return TOKEN_WHILE; }
"read"      { return TOKEN_READ; }
"print"     { return TOKEN_PRINT; }
"return"    { return TOKEN_RETURN; }

[a-zA-Z][a-zA-Z0-9_]* { yylval.strVal = strdup(yytext); return TOKEN_ID; }
[0-9]+\.[0-9]+        { yylval.floatVal = atof(yytext); return TOKEN_NUM_FLOAT; }
[0-9]+                { yylval.intVal = atoi(yytext); return TOKEN_NUM_INT; }

":="        { return TOKEN_ASSIGN; }
"=="        { return TOKEN_EQ; }
"!="        { return TOKEN_NEQ; }
"."         { return TOKEN_DOT; }
","         { return ','; }
"+"         { return '+'; }
"-"         { return '-'; }
"*"         { return '*'; }
"/"         { return '/'; }
"%"         { return '%'; }
"^"         { return '^'; }
"("         { return '('; }
")"         { return ')'; }
">"         { return '>'; }
"<"         { return '<'; }

[ \t]+      ;
\n          ; 
"#"[^\n]* ; 
.           { printf("Satir %d: Bilinmeyen karakter: %s\n", yylineno, yytext); }

%%
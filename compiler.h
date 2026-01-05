#ifndef COMPILER_H
#define COMPILER_H
#include "ast.h"

int semantic_analysis(ASTNode* node);
void generate_code(ASTNode* node, const char* filename);

#endif
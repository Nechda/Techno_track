#pragma once
#include "Types.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __GNUG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#define Assert_c(expr) if(!(expr)) printf("Expression %s is false.\nIn file: %s\nfunction: %s\nline: %d\n", #expr, __FILE__, __FUNCSIG__, __LINE__);


struct Node
{
    char* data;
    Node* l;
    Node* r;
};

typedef FILE* Stream;

struct Tree
{
    Node* root = NULL;
    void init(const C_string filename);
    void destr();
    void print(Stream stream = stdout);
    void draw(const char* filename = "tree.dot");
    void getDefinition(const C_string word);
};
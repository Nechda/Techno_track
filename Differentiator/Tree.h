#pragma once
#include <stdio.h>
#include "Types.h"
#include "Logger.h"
#include "CallStack.h"

typedef FILE* Stream;



#define DOT_LEAF_STYLE "[shape = \"ellipse\", fillcolor = \"lightgreen\"]"
#define DOT_NODE_STYLE "[color = \"black\", fontsize = 24, shape = \"box\", style = \"filled, rounded\", fillcolor = \"lightgray\"]"
#define DOT_EDGE_STYLE "[color = \"black\", fontsize = 24]"


template <class Type>
struct Node
{
    Type* ptrToData;
    Node<Type>* link[2];
    Node<Type>* parent;
};

template <class Type>
void rCleanUp(Node<Type>* node)
{$
    if (!node) { $$ return; }
    if (node->ptrToData) free(node->ptrToData);
    rCleanUp(node->link[0]);
    rCleanUp(node->link[1]);
    free(node);
    $$
}

template <class Type>
Node<Type>* rCopy(const Node<Type>* node,Node<Type>* parent = NULL)
{
    $
    if (!node)
    {
        $$$("NULL ptr node");
        return NULL;
    }
    Node<Type>* res = (Node<Type>*)calloc(1, sizeof(Node<Type>));
    *res = *node;
    res->ptrToData = (Type*)calloc(1, sizeof(Type));
    *(res->ptrToData) = *(node->ptrToData);
    res->link[0] = rCopy(node->link[0], res);
    res->link[1] = rCopy(node->link[1], res);
    res->parent = parent;
    $$
    return res;
}


template <class Type>
class Tree
{
    public:
        typedef Node<Type> TNode;
    protected:
        virtual void printNodeInDotFile(TNode* node, Stream stream)  = 0;
        virtual void writeTreeInFile(TNode* node, ui32 level, Stream stream) = 0;
        virtual void readTreeFromFile(const C_string filename) = 0;
        inline void setRoot(TNode* ptr);
        inline TNode* getRoot();
        bool isValid = 0;
        TNode ground;
    public:
        Tree() {};
        Tree(const C_string filename);
        ~Tree();
        void genTreeByRoot(TNode* node)
        {
            $
            if (isValid)
                return;
            ground.link[0] = node;
            node->parent = &ground;
            $$
        }
        void print(Stream stream = stdout);
        void drawGraph(const C_string outFilename = "tree"); ///<имя файла нужно указывать без расширения!!!
};

template <class Type>
inline Node<Type>* Tree<Type>::getRoot()
{
    return ground.link[0];
}

template <class Type>
inline void Tree<Type>::setRoot(TNode* ptr)
{
    ground.link[0] = ptr;
}

template <class Type>
Tree<Type>::Tree(const C_string filename)
{$
    if (isValid)
    {
        $$$("Tree structure is invalid.");
        return;
    }

    ground.link[0] = (Node<Type>*)calloc(1, sizeof(Node<Type>));
    ground.link[1] = NULL;
    ground.ptrToData = NULL;
    ground.parent = NULL;

    Assert_c(ground.link[0]);
    if (!ground.link[0])
    {
        $$$("Can't allocate memory for root.");
        return;
    }
    getRoot()->parent = &ground;
    $$
}

template <class Type>
Tree<Type>::~Tree()
{$  
    isValid = 0;
    rCleanUp(ground.link[0]);
    ground.link[0] = NULL;
    $$
}

template <class Type>
void Tree<Type>::print(Stream stream = stdout)
{$
    printf("Hey hey hey, are you sure, that you really need this function?\n"
           "This function can't work with standard functions, so if you don't want your computer will burn\n"
           "don't use this function anymore");
    $$
    return;
    writeTreeInFile(getRoot(), 0, stream);
    if (stream == stdout)
        printf("\n");
    $$
}

template <class Type>
void Tree<Type>::drawGraph(const C_string outFilename)
{$
    char buffer[256];
    Assert_c(outFilename);
    if (!outFilename)
    {
        $$$("NULL ptr in C_string outFilename")
        return;
    }
    sprintf(buffer, "%s.dot", outFilename);
    FILE* file = fopen(buffer, "w");
    Assert_c(file);
    if (!file)
    {
        $$$("NULL ptr in FILE* file");
        return;
    }

    fprintf(file,
        "digraph{\n"
        "node %s\n"
        "edge %s\n"
        , DOT_NODE_STYLE, DOT_EDGE_STYLE
    );
    printNodeInDotFile(getRoot(), file);
    fprintf(file,
        "}"
    );

    fclose(file);

    sprintf(buffer, "dot -Tpng %s.dot -o %s.png", outFilename, outFilename);
    system(buffer);
    $$
}
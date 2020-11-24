#pragma once
#include <stdio.h>
#include "Types.h"
#include "Logger.h"

typedef FILE* Stream;



#define DOT_LEAF_STYLE "[shape = \"ellipse\", fillcolor = \"lightgreen\"]"
#define DOT_NODE_STYLE "[color = \"black\", fontsize = 24, shape = \"box\", style = \"filled, rounded\", fillcolor = \"lightgray\"]"
#define DOT_EDGE_STYLE "[color = \"black\", fontsize = 24]"


template <class Type>
struct Node
{
    Type* data;
    Node<Type>* link[2];
    Node<Type>* parent;
    void cleanUp()
    {
        if (!data)
            return;
        for(int i=0;i<2;i++)
        if (link[i])
        {
            link[i]->cleanUp();
            free(link[i]);
        }
        free(data);
    }
};



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
    private:
        TNode* ground = NULL;
    public:
        Tree() {};
        Tree(const C_string filename);
        ~Tree();
        void print(Stream stream = stdout);
        void drawGraph(const C_string outFilename = "tree.dot");
};

template <class Type>
inline Node<Type>* Tree<Type>::getRoot()
{
    return ground->link[0];
}

template <class Type>
inline void Tree<Type>::setRoot(TNode* ptr)
{
    ground->link[0] = ptr;
}

template <class Type>
Tree<Type>::Tree(const C_string filename)
{
    if (isValid)
        return;
    Assert_c(!ground);
    if (ground)
        return;

    ground = (Node<Type>*)calloc(1, sizeof(Node<Type>));
    ground->link[0] = (Node<Type>*)calloc(1, sizeof(Node<Type>));
    Assert_c(ground || ground->link[0]);
    if (!ground && !ground->link[0])
        return;
    getRoot()->parent = ground;
    isValid = 1;
}

template <class Type>
Tree<Type>::~Tree()
{
    isValid = 0;
    ground->cleanUp();
    if (ground)
    {
        free(ground);
        ground = NULL;
    }
}

template <class Type>
void Tree<Type>::print(Stream stream = stdout)
{
    writeTreeInFile(getRoot(), 0, stream);
}

template <class Type>
void Tree<Type>::drawGraph(const C_string outFilename)
{
    FILE* file = fopen(outFilename, "w");
    Assert_c(file);
    if (!file)
        return;
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
    //system("dot -Tpng tree.dot -o tree.png");
}
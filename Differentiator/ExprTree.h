#pragma once
#include "Tree.h"
#include "Types.h"

enum ExpType
{
    EXP_UNDEFINED,
    EXP_NUMBER,
    EXP_OPERATION,
    EXP_VARIABLE
};


struct ExpInfo
{
    C_string str;
    ExpType type;
    ui32 value;
};

class Expression : public Tree<ExpInfo>
{
private:
    void printNodeInDotFile(TNode* node, Stream stream);
    void writeTreeInFile(TNode* node, ui32 level, Stream stream);
    void readTreeFromFile(const C_string filename);
public:
    Expression() {};
    Expression(const C_string filename) : Tree(filename)
    {
        readTreeFromFile(filename);
    };
    ~Expression() {};
    void simplify();
};

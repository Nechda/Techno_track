#pragma once
#include "Tree.h"
#include "Types.h"

enum NodeType
{
    EXP_UNDEFINED,
    EXP_NUMBER,
    EXP_OPERATION,
    EXP_VARIABLE,
    EXP_FUNCTION
};

enum OperationType
{
    OP_SUM,
    OP_SUB,
    OP_MUL,
    OP_DIV
};

struct NodeInfo
{
    NodeType type;
    union
    {
        ui32 opNumber;
        double number;
    }data;
};

class Expression : public Tree<NodeInfo>
{
    private:
        void printNodeInDotFile(TNode* node, Stream stream);
        void writeTreeInFile(TNode* node, ui32 level, Stream stream);
        void readTreeFromFile(const C_string filename);
    public:
        Expression() {};
        Expression(const Expression& exp);
        Expression(const C_string filename);
        ~Expression() {};
        void simplify();
        void genTex(Stream stream = stdout);
};

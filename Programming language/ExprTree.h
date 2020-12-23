#pragma once
#include "Tree.h"
#include "Types.h"
#include <map>

enum NodeType
{
    NODE_TYPE_UNDEFINED,
    NODE_TYPE_NUMBER,
    NODE_TYPE_OPERATION,
    NODE_TYPE_VARIABLE,
    NODE_TYPE_VARIABLE_SPECIFICALOR,
    NODE_TYPE_FUNCTION
};

enum OpType
{
    #define OP_DEFINE(string, name, enumName, enumToken, priority, implentation)\
        enumName,
        #include "OPERATORS.h"
    #undef OP_DEFINE
    OP_UNDEFINED = -1
};

enum FuncType
{
    #define FUNC_DEFINE(name, enumName, implentation)\
        enumName,
        #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
};

const C_string FUNCTION_NAMES_TABLE[] =
{
    #define FUNC_DEFINE(name, enumName, implentation)\
        #name,
        #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
};
const ui32 FUNCTION_TABLE_SIZE = sizeof(FUNCTION_NAMES_TABLE) / sizeof(FUNCTION_NAMES_TABLE[0]);

typedef union
{
    ui32   ivalue;
    double dvalue;
}UnionData;



struct NodeInfo
{
    NodeType type;
    UnionData dataUnion;
};


class Expression : public Tree<NodeInfo>
{
    private:
        std::map<ui64, double> varables;
        void printNodeInDotFile(TNode* node, Stream stream);
        void readTreeFromFile(const C_string filename);
    public:
        Expression() {};
        Expression(const Expression& exp);
        Expression(const C_string filename);
        ~Expression() { varables.clear(); };
        void simplify();
        double evaluate();
        bool isValidStruct();
};

Expression::TNode* createNode(Expression::TNode* left, Expression::TNode* right, NodeType typeNode, UnionData unionData, Expression::TNode* parent);
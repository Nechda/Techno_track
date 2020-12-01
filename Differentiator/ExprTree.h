#pragma once
#include "Tree.h"
#include "Types.h"

enum NodeType
{
    NODE_TYPE_UNDEFINED,
    NODE_TYPE_NUMBER,
    NODE_TYPE_OPERATION,
    NODE_TYPE_VARIABLE,
    NODE_TYPE_FUNCTION
};

enum OpType
{
    #define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)
    #define OP_DEFINE(symbol, name, enumName, priority, implentation,texPrintImplemenation)\
        enumName,
    #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
    #undef OP_DEFINE
};

enum FuncType
{
    #define OP_DEFINE(name, enumName, implentation, texPrintImplemenation)
    #define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)\
        enumName,
        #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
    #undef OP_DEFINE
};

const C_string FUNCTION_NAMES_TABLE[] =
{
    #define OP_DEFINE(name, enumName, implentation, texPrintImplemenation)
    #define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)\
        #name,
        #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
    #undef OP_DEFINE
};
const ui32 FUNCTION_TABLE_SIZE = sizeof(FUNCTION_NAMES_TABLE) / sizeof(FUNCTION_NAMES_TABLE[0]);

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
        double evaluate(double variable);
        void genTex(Stream stream = stdout);
        void genTexFile(const C_string outFilename);
        void differentiate();
        bool isValidStruct();
};

Expression::TNode* createNode(Expression::TNode* left, Expression::TNode* right, NodeType typeNode, double value, Expression::TNode* parent);
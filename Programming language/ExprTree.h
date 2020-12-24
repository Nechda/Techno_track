#pragma once
#include "Types.h"
#include "Tree.h"
#include "Hash.h"
#include <map>
#include <stack>


using std::pair;
using std::map;
using std::stack;

enum NodeType
{
    NODE_TYPE_UNDEFINED,
    NODE_TYPE_NUMBER,
    NODE_TYPE_OPERATION,
    NODE_TYPE_NAME,
    NODE_TYPE_VARIABLE_SPECIFICALOR,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_CUSTOM_FUNCTION
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

enum EvaluationErrors
{
    STATUS_OK,
    ERR_INVALID_TREE_STRUCTURE,
    ERR_ATTEMPT_REDEFINE_FUNCTION_TABLE,
    ERR_UNDEFINED_VARIABLE,
    ERR_REDIFINITION_VARIABLE,
    ERR_UNDEFINED_FUNCTION,
    ERR_REDIFINITION_FUNCTION,
    ERR_FUNCTION_PARAM_REDEFINITION,
    ERR_INVALID_NUMBER_OF_FUNCTION_ARGUMENTS,
    ERR_FORGOTTEN_RETURN_OPERATOR,
};

class Expression : public Tree<NodeInfo>
{
    private:
        static const pair<EvaluationErrors, const C_string> errorExplanationTable[];
        static const Hash entryPointHash;
        stack<double> programStack;
        bool isCurrentFunctionCalledReturn;
        EvaluationErrors errorCode;

        void printNodeInDotFile(TNode* node, Stream stream);
        void readTreeFromFile(const C_string filename);

        map<Hash, TNode*> functionsTable;
        EvaluationErrors genFunctionTable();
        void customFunctionEvaluate(Hash functionHash, stack<double>& argc);
        double treeEvaluate(TNode* node, map<Hash, pair<double, ui32>>& variables);
    public:
        Expression() {};
        Expression(const Expression& exp);
        Expression(const C_string filename);
        ~Expression() { functionsTable.clear(); };
        void simplify();
        void evaluate();
        bool isValidStruct();
        void getEvaluateStatus();
        
};

Expression::TNode* createNode(Expression::TNode* left, Expression::TNode* right, NodeType typeNode, UnionData unionData, Expression::TNode* parent);
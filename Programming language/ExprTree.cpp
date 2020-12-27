#include "ExprTree.h"
#include "LibStr.h"
#include "Logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <clocale>
#include <stack>
#include "Parser.h"
#include "CallStack.h"

const Hash Expression::entryPointHash = getHash("main", 4);
const pair<CompilatorError, const C_string> Expression::errorExplanationTable[] =
{
    {STATUS_OK,                                 "Status Ok"},
    {ERR_INVALID_TREE_STRUCTURE,                "Tree structure is invalid"},
    {ERR_ATTEMPT_REDEFINE_FUNCTION_TABLE,       "Try to redefine function table"},
    {ERR_UNDEFINED_VARIABLE,                    "Try to access to undefined variable"},
    {ERR_REDIFINITION_VARIABLE,                 "Try to redefine already existed variable"},
    {ERR_UNDEFINED_FUNCTION,                    "Try to access to undefined function"},
    {ERR_REDIFINITION_FUNCTION,                 "Try to redefine already existed function"},
    {ERR_FUNCTION_PARAM_REDEFINITION,           "Function param redefinition"},
    {ERR_INVALID_NUMBER_OF_FUNCTION_ARGUMENTS,  "Invalid number of function params"},
    {ERR_FORGOTTEN_RETURN_OPERATOR,             "The ret operator was expected, but there is not"},
    {ERR_NO_ENTRY_POINT_FUNCTION,               "There isn't entry point function. By default it's def main()"}
};

static const float ACCURACY = 1E-3;

inline bool isZero(float num)
{
    return fabs(num) < ACCURACY;
}

inline bool isInteger(float num)
{
    return isZero(num - ceil(num));
}

inline void setLinkParent(Expression::TNode*& node)
{
    if (node->link[0]) node->link[0]->parent = node;
    if (node->link[1]) node->link[1]->parent = node;
}

/*
    \brief Функция, отвечающая за преобразование узла дерева в строку
    \note  При добавлении нового элемента в язык, требуется переписать данную функцию
           в противном случае выводить дерево в файл не получится
*/
C_string expInfoToStr(NodeInfo* exp, bool printAddr = false)
{$
    if (!exp)
        return "";
    static char buffer[64] = {};
    static C_string opDict[] =
    {
        #define OP_DEFINE(string, name, enumName, priority, implentation, canUseInConstantSimplify, asmCodeTranslator)\
                string,
        #include "OPERATORS.h"
        #undef OP_DEFINE
    };
    ui8 offset = 0;
    bool isIntNumber = isInteger(exp->dataUnion.dvalue);
    switch (exp->type)
    {
        case NODE_TYPE_NUMBER:
            if(isIntNumber)
                sprintf(buffer,"%d",(int)exp->dataUnion.dvalue);
            else
                sprintf(buffer, "%.3lf", exp->dataUnion.dvalue);
            break;
        case NODE_TYPE_OPERATION:
            sprintf(buffer, "%s", opDict[exp->dataUnion.ivalue]);
            break;
        case NODE_TYPE_NAME:
            sprintf(buffer, "Hash(name):\n 0x%X", exp->dataUnion.ivalue);
            break;
        case NODE_TYPE_CUSTOM_FUNCTION:
            sprintf(buffer, "func");
            break;
        case NODE_TYPE_VARIABLE_SPECIFICALOR:
            sprintf(buffer, "var");
            break;
        case NODE_TYPE_FUNCTION:
            sprintf(buffer, "%s", FUNCTION_NAMES_TABLE[exp->dataUnion.ivalue]);
            break;
        default:
            $$$("We can't parse this ExpInfo structure.");
            return buffer;
            break;
    }

    if (printAddr)
    {
        while (buffer[offset])
            offset++;
        offset = offset >= 64 ? 0 : offset;
        sprintf(buffer + offset, "\n0x%X", exp);
    }
    $$
    return buffer;
}


bool Expression::isValidStruct()
{
    return isValid;
}

void Expression::getEvaluateStatus()
{
    printf("%s\n", errorExplanationTable[static_cast<ui32>(errorCode)].second);
}

Expression::Expression(const Expression& exp)
{$
    setRoot(rCopy<NodeInfo>(exp.ground.link[0]));
    $$ return;
}

Expression::Expression(const C_string filename) : Tree(filename)
{$
    readTreeFromFile(filename);
    $$ return;
};


//===============================================================================================================================
//|                                                                                                                             |
//|                                                                                                                             |
//|                                 чтение дерева из файла + генерация дерева в dot файл                                        |
//|                                                                                                                             |
//|_____________________________________________________________________________________________________________________________|
//|                                                                                                                             |

/*
    \brief Функция генерации dot файла для graphviz
*/
void Expression::printNodeInDotFile(TNode* node, Stream stream)
{$
    Assert_c(stream);
    if (!stream)
    {
        $$$("NULL ptr in node or stream structure");
        return;
    }
    if (!node)
    {
        $$ return;
    }

    bool isLeaf = 1;
    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
        isLeaf &= !node->link[i];

    if (isLeaf)
    {
        fprintf(stream,
            "\"%s\" %s \n", expInfoToStr(node->ptrToData, true), DOT_LEAF_STYLE
        );
    }
    else
    {
        for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
            printNodeInDotFile(node->link[i], stream);


        fprintf(stream, " \"%s\" \n", expInfoToStr(node->ptrToData,true));
        for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
            if (node->link[i])
            {
                fprintf(stream, " \"%s\" -> ", expInfoToStr(node->ptrToData, true));
                fprintf(stream, " \"%s\" [label = \"L[%d]\", fontsize = 14] \n", expInfoToStr(node->link[i]->ptrToData, true), i);
            }
    }
    $$
}

/*
    \brief Функция генерирует дерево из файла
    \detail Функция считывает файл, затем содержимое
            отправляет парсеру, который строит дерево.
*/
void Expression::readTreeFromFile(const C_string filename)
{$
    Assert_c(filename);
    if (!filename || isValid)
    {
        $$$("invalid tree structure or filename is NULL");
        return;
    }

    char* buffer = NULL;
    int errorCode = 0;
    errorCode = readFullFile(filename, &buffer);
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(getRoot());
        setRoot(NULL);
        $$$("readFullFile has returned error");
        return;
    }

    errorCode = removeExtraChar(&buffer, "(+-*/^.,) =;{}&|<>!?[]");
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(getRoot());
        setRoot(NULL);
        $$$("removeExtraChar has returned error");
        return;
    }

    C_string str = buffer;
    Parser pr;
    Expression::TNode* newRoot = pr.parse(str);
    if (newRoot)
    {
        genTreeByRoot(newRoot);
        isValid = 1;
    }
    else
        isValid = 0;
    $$
}
//|_____________________________________________________________________________________________________________________________|
//===============================================================================================================================



//===============================================================================================================================
//|                                                                                                                             |
//|                                                                                                                             |
//|                                  реализация алгоритмов упрощения дерева                                                     |
//|                                                                                                                             |
//|                                                                                                                             |
//|_____________________________________________________________________________________________________________________________|
//|                                                                                                                             |
typedef double(*FuncionType)(double, double);


#define OP_DEFINE(string, name, enumName, priority, implentation, canUseInConstantSimplify, asmCodeTranslator)\
    static double runEvaluate##name(double left, double right) implentation
#define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)\
    static double runEvaluate##name(double left, double right) implentation
#include "FUNCTIONS.h"
#include "OPERATORS.h"
#undef OP_DEFINE
#undef FUNC_DEFINE



FuncionType evaluateFunctions[] = {
    #define OP_DEFINE(string, name, enumName, priority, implentation, canUseInConstantSimplify, asmCodeTranslator)\
        runEvaluate##name,
    #include "OPERATORS.h"
    #undef OP_DEFINE
};

bool evaluateFunctionsPreprocessor[] = {
    #define OP_DEFINE(string, name, enumName, priority, implentation,canUseInConstantSimplify)\
            canUseInConstantSimplify,
    #include "OPERATORS.h"
    #undef OP_DEFINE
};

FuncionType evaluateStandartFunctions[] =
{
    #define FUNC_DEFINE(name, enumName, implentation, canUseInConstantSimplify, asmCodeTranslator)\
        runEvaluate##name,
    #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
};

bool evaluateStandartFunctionsPreprocessor[] =
{
    #define FUNC_DEFINE(name, enumName, implentation, canUseInConstantSimplify)\
            canUseInConstantSimplify,
    #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
};


static bool constantSimplify(Expression::TNode* node, bool& isChangedTree)
{
    if (!node)
        return true;
    $
    bool isLeaf = 1;
    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
        isLeaf &= !node->link[i];
    if (isLeaf)
    {
        $$
        return node->ptrToData->type == NODE_TYPE_NUMBER;
    }


    bool canEvaluate = 1;
    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
        canEvaluate &= constantSimplify(node->link[i], isChangedTree);
    if (!canEvaluate) { $$; return false; }


    #define getNum(p) (p) ? (p)->ptrToData->dataUnion.dvalue : 0
    double num[2] = {};
    num[0] = getNum(node->link[0]);
    num[1] = getNum(node->link[1]);
    #undef getNum

    NodeInfo nodeInfo = *node->ptrToData;
    
    FuncionType* evauateFunction = nodeInfo.type == NODE_TYPE_OPERATION ? evaluateFunctions : evaluateStandartFunctions;
    bool canChangeTree = nodeInfo.type == NODE_TYPE_OPERATION ? evaluateFunctionsPreprocessor[nodeInfo.dataUnion.ivalue] : evaluateStandartFunctionsPreprocessor[nodeInfo.dataUnion.ivalue];

    //может ли данная функция изменять дерево на этапе препроцессора?
    if (!canChangeTree)
        return false;

    node->ptrToData->dataUnion.dvalue = evauateFunction[nodeInfo.dataUnion.ivalue](num[0], num[1]);
    node->ptrToData->type = NODE_TYPE_NUMBER;

    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
    {
        rCleanUp(node->link[i]);
        node->link[i] = NULL;
    }

    isChangedTree |= 1;
    $$
    return true;
}

static void identitySimplify(Expression::TNode* node, bool& isChangedTree)
{
    if (!node)
        return;
    $

    bool isLeaf = 1;
    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
        isLeaf &= !node->link[i];
    if (isLeaf) { $$;  return; }

    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
        identitySimplify(node->link[i], isChangedTree);

    
    #define isZeroNumber(p) isZero(p->ptrToData->dataUnion.dvalue - 0) && p->ptrToData->type == NODE_TYPE_NUMBER
    #define isOneNumber(p)  isZero(p->ptrToData->dataUnion.dvalue - 1) && p->ptrToData->type == NODE_TYPE_NUMBER

    #define replaceLink(node_, oldLink_, newLink_)              \
        if((node_))                                             \
        for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)             \
            if ((node_)->link[i] == (oldLink_) && (node_))      \
                (node_)->link[i] = (newLink_);                    

    if (node->ptrToData->type == NODE_TYPE_OPERATION)
    {
        Expression::TNode* parent = NULL;
        Expression::TNode* newLink = NULL;
        Expression::TNode* tmpLink = NULL;
        UnionData unionData;
        unionData.ivalue = 0;
        switch (node->ptrToData->dataUnion.ivalue)
        {
        case OP_SUM:
            // x + 0 = 0 + x = x
            for (int j = 0; j < 2; j++)
                if (isZeroNumber(node->link[j]))
                {
                    parent = node->parent;
                    newLink = node->link[j ^ 1];
                    newLink->parent = parent;
                    replaceLink(parent, node, newLink);

                    rCleanUp(node->link[j]);
                    free(node->ptrToData);
                    free(node);
                    isChangedTree |= 1;
                    $$
                    return;
                }
            break;
        case OP_SUB:
            
            // x - 0 = x
            if (isZeroNumber(node->link[1]))
            {
                parent = node->parent;
                newLink = node->link[0];
                newLink->parent = parent;
                replaceLink(parent, node, newLink);

                rCleanUp(node->link[1]);
                free(node->ptrToData);
                free(node);
                isChangedTree |= 1;
                $$
                    return;
            }

            // 0 - x = -x
            if (isZeroNumber(node->link[0]))
            {
                rCleanUp(node->link[0]);
                node->link[0] = node->link[1];
                node->link[1] = NULL;
                node->ptrToData->type = NODE_TYPE_FUNCTION;
                node->ptrToData->dataUnion.ivalue = FUNC_NEG;
                isChangedTree |= 1;
                $$
                return;
            }
            break;
        case OP_MUL:

            // x * 0 = 0 * x = 0
            for (int j = 0; j < 2; j++)
                if (isZeroNumber(node->link[j]))
                {
                    parent = node->parent;
                    unionData.dvalue = 0.0;
                    newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, unionData, parent);
                    replaceLink(parent, node, newLink);

                    rCleanUp(node);
                    isChangedTree |= 1;
                    $$
                    return;
                }

            // x * 1 = 1 * x = x
            for (int j = 0; j < 2; j++)
                if (isOneNumber(node->link[j]))
                {
                    parent = node->parent;
                    newLink = node->link[j ^ 1];
                    newLink->parent = parent;
                    replaceLink(parent, node, newLink);

                    rCleanUp(node->link[j]);
                    free(node->ptrToData);
                    free(node);
                    isChangedTree |= 1;
                    $$
                    return;
                }
            break;
        case OP_DIV:

            if (isOneNumber(node->link[1]))
            {
                parent = node->parent;
                newLink = node->link[0];
                newLink->parent = parent;
                replaceLink(parent, node, newLink);

                rCleanUp(node->link[1]);
                free(node->ptrToData);
                free(node);
                isChangedTree |= 1;
                $$
                return;
            }
            break;
        case OP_POW:

            //x^1 -> x
            if (isOneNumber(node->link[1]))
            {
                parent = node->parent;
                newLink = node->link[0];
                newLink->parent = parent;
                replaceLink(parent, node, newLink);

                rCleanUp(node->link[1]);
                free(node->ptrToData);
                free(node);
                isChangedTree |= 1;
                $$
                return;
            }


            //x^0 -> 1
            if (isZeroNumber(node->link[1]))
            {
                parent = node->parent;
                unionData.dvalue = 1.0;
                newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, unionData, parent);
                replaceLink(parent, node, newLink);

                rCleanUp(node);
                isChangedTree |= 1;
                $$
                return;
            }

            //1^x -> 1
            if (isOneNumber(node->link[0]))
            {
                parent = node->parent;
                unionData.dvalue = 1.0;
                newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, unionData, parent);
                replaceLink(parent, node, newLink);

                rCleanUp(node);
                isChangedTree |= 1;
                $$
                return;
            }


            //0^x -> 0
            if (isZeroNumber(node->link[0]))
            {
                parent = node->parent;
                unionData.dvalue = 0.0;
                newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, unionData, parent);
                replaceLink(parent, node, newLink);

                rCleanUp(node);
                isChangedTree |= 1;
                $$
                    return;
            }


            // (E^a)^b = E^(a+b)
            if (node->link[0]->ptrToData->type == NODE_TYPE_OPERATION
            &&  node->link[0]->ptrToData->dataUnion.ivalue == OP_POW
                )
            {
                parent = node->parent;
                unionData.ivalue = OP_SUM;
                newLink = createNode(node->link[0]->link[1], node->link[1], NODE_TYPE_OPERATION, unionData, node);
                tmpLink = node->link[0]->link[0];
                free(node->link[0]->ptrToData);
                free(node->link[0]);

                node->link[0] = tmpLink;
                node->link[1] = newLink;

                setLinkParent(node);
                setLinkParent(node->link[0]);
                setLinkParent(node->link[1]);

                isChangedTree |= 1;
                $$
                    return;
            }

            break;
        default:
            break;
        }
    }


    #undef isZeroNumber(p)
    #undef isOneNumber(p)
    #undef replaceLink(node_, oldLink_, newLink_)
    $$
}

void Expression::simplify()
{$
    Assert_c(isValid);
    if (!isValid)
    {
        $$$("Invalid structure");
        return;
    }
    bool isChangedTree = 0;
    do
    {
        isChangedTree = 0;
        constantSimplify(getRoot(), isChangedTree);
        identitySimplify(getRoot(), isChangedTree);
    } while (isChangedTree);
    $$
}
//|_____________________________________________________________________________________________________________________________|
//===============================================================================================================================


//===============================================================================================================================
//|                                                                                                                             |
//|                                                                                                                             |
//|                                                 вычисление дерева                                                           |
//|                                                                                                                             |
//|                                                                                                                             |
//|_____________________________________________________________________________________________________________________________|
//|                                                                                                                             |
double Expression::treeEvaluate(Expression::TNode* node, map<Hash, pair<double,ui32>>& variables)
{
    if (!node || isCurrentFunctionCalledReturn || errorCode)
        return 0;
    $
    bool isLeaf = 1;
    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
        isLeaf &= !node->link[i];

    isLeaf &= !(node->ptrToData->type == NODE_TYPE_FUNCTION || node->ptrToData->type == NODE_TYPE_CUSTOM_FUNCTION);

    ui64 variableHash = 0;
    if (isLeaf){
        variableHash = node->ptrToData->dataUnion.ivalue;
        if (!variables.count(variableHash) && node->ptrToData->type != NODE_TYPE_NUMBER){
            errorCode = ERR_UNDEFINED_VARIABLE;
            $$$("Undefined variable!");
            return 0;
        }
        $$ return node->ptrToData->type == NODE_TYPE_NUMBER ? node->ptrToData->dataUnion.dvalue : variables[variableHash].first;
    }

    FuncionType* evauateFunction;
    NodeType nodeType = node->ptrToData->type;
    double result = 0;
    double num[2] = {};
    if(nodeType == NODE_TYPE_OPERATION)
        switch (node->ptrToData->dataUnion.ivalue){
            case OP_SEMICOLON:
                treeEvaluate(node->link[0], variables);
                treeEvaluate(node->link[1], variables);
                break;
            case OP_ASSIGMENT:
                //проверка, что link[0] --- переменная
                if (node->link[0]->ptrToData->type == NODE_TYPE_NAME){
                    variableHash = node->link[0]->ptrToData->dataUnion.ivalue;
                    if (!variables.count(variableHash)){
                        errorCode = ERR_UNDEFINED_VARIABLE;
                        $$$("Undefined variable!");
                        return 0;
                    }
                }
                else if (node->link[0]->ptrToData->type == NODE_TYPE_VARIABLE_SPECIFICALOR){
                    variableHash = node->link[0]->link[0]->ptrToData->dataUnion.ivalue;
                    if (variables.count(variableHash)){
                        errorCode = ERR_REDIFINITION_VARIABLE;
                        $$$("Redefinition variable!");
                        return 0;
                    }
                    variables[variableHash].first = 0;
                }
                result = treeEvaluate(node->link[1], variables);
                variables[variableHash].first = result;
                break;
            case OP_BRANCH:
                result = treeEvaluate(node->link[0], variables);
                if (result)
                    treeEvaluate(node->link[1], variables);
                else
                    treeEvaluate(node->link[2], variables);
                break;
            case OP_WHILE:
                result = treeEvaluate(node->link[0], variables);
                while (result)
                {
                    treeEvaluate(node->link[1], variables);
                    result = treeEvaluate(node->link[0], variables);
                }
                break;
            case OP_RETURN:
                programStack.push(treeEvaluate(node->link[0], variables));
                isCurrentFunctionCalledReturn |= 1;
                break;
            default:
                num[0] = treeEvaluate(node->link[0], variables);
                num[1] = treeEvaluate(node->link[1], variables);
                result = evaluateFunctions[node->ptrToData->dataUnion.ivalue](num[0], num[1]);
                break;
        }
    if (nodeType == NODE_TYPE_FUNCTION){
        num[0] = treeEvaluate(node->link[0], variables);
        num[1] = treeEvaluate(node->link[1], variables);
        result = evaluateStandartFunctions[node->ptrToData->dataUnion.ivalue](num[0], num[1]);
    }

    if (nodeType == NODE_TYPE_CUSTOM_FUNCTION){
        Hash functionNameHash = node->link[0]->ptrToData->dataUnion.ivalue;
        //производим вычисление аргументов, а результат запихиваем в стек программы
        stack<double>& arguments = programStack;
        TNode* argNode = node->link[1];
        #define isNotEnd() argNode->ptrToData->type == NODE_TYPE_OPERATION && argNode->ptrToData->dataUnion.ivalue == OP_COMMA
        if (argNode)
        {
            while (isNotEnd())
            {
                arguments.push(treeEvaluate(argNode->link[0], variables));
                argNode = argNode->link[1];
            } 
            arguments.push(treeEvaluate(argNode, variables));
        }
        #undef isNotEnd()
        //запускаем выполнение функции
        customFunctionEvaluate(functionNameHash, arguments);
        if(programStack.empty()){
            errorCode = ERR_FORGOTTEN_RETURN_OPERATOR;
            $$$("Program stack is empty, you forgotten ret operator.");
            return 0;
        }
        result = programStack.top();
        programStack.pop();
    }

    $$ return result;
}

static CompilatorError genFuncTableInteration(Expression::TNode* node, map<Hash, Expression::TNode*>& functionsTable)
{$
    if(!node){
        $$ return STATUS_OK;
    }

    bool isFunctionUnification = 0;
    isFunctionUnification = node->ptrToData->type == NODE_TYPE_OPERATION 
                         && node->ptrToData->dataUnion.ivalue == OP_DOLLAR;
    CompilatorError errorCode = STATUS_OK;

    if (isFunctionUnification){
        errorCode = genFuncTableInteration(node->link[1], functionsTable);
        node = node->link[0];
    }
    if (errorCode){
        $$ return errorCode;
    }


    if (functionsTable.count(node->link[0]->ptrToData->dataUnion.ivalue)){
        $$$("Function redefinition!");
        return ERR_REDIFINITION_FUNCTION;
    }
    functionsTable[node->link[0]->ptrToData->dataUnion.ivalue] = node;


    $$ return errorCode;
}

CompilatorError Expression::genFunctionTable()
{$
    Assert_c(isValid);
    if (!isValid){
        $$$("Invalid structure.");
        return ERR_INVALID_TREE_STRUCTURE;
    }

    if (functionsTable.size()){
        $$$("The table has generated already.");
        return ERR_ATTEMPT_REDEFINE_FUNCTION_TABLE;
    }

    TNode* node = getRoot();
    $$ return genFuncTableInteration(node, functionsTable);
}

static CompilatorError genVarTableInteration(Expression::TNode* node, map<Hash, pair<double, ui32>>& varablesTable, stack<double>& argc)
{$
    if (!node){
        $$ return STATUS_OK;
    }

    bool isVariableUnification = 0;
    isVariableUnification = node->ptrToData->type == NODE_TYPE_OPERATION
        && node->ptrToData->dataUnion.ivalue == OP_COMMA;
    CompilatorError errorCode = STATUS_OK;

    if (isVariableUnification){
        errorCode = genVarTableInteration(node->link[1], varablesTable, argc);
        node = node->link[0];
    }

    node = node->link[0];
    if (varablesTable.count(node->ptrToData->dataUnion.ivalue)){
        $$$("Function param redefinition!");
        return ERR_FUNCTION_PARAM_REDEFINITION;
    }
    if (!argc.empty()){
        varablesTable[node->ptrToData->dataUnion.ivalue] = pair<double, ui32>(argc.top(), 0);
        argc.pop();
    }else{
        $$$("Invalid number of arguments!");
        return ERR_INVALID_NUMBER_OF_FUNCTION_ARGUMENTS;
    }

    $$ return errorCode;
}

void Expression::customFunctionEvaluate(Hash functionHash, stack<double>& argc)
{$
    if (!functionsTable.count(functionHash)){
        errorCode = ERR_UNDEFINED_FUNCTION;
        printf("Undefined function hash: 0x%X\n", functionHash);
        $$$("Undefined function.");
        return;
    }

    TNode* node = functionsTable[functionHash];
    map<Hash, pair<double, ui32>> varablesTable;
    errorCode = genVarTableInteration(node->link[1], varablesTable, argc);
    if (errorCode) {
        $$ return;
    }
    treeEvaluate(node->link[2], varablesTable);
    isCurrentFunctionCalledReturn = 0;
    $$ return;
}

void Expression::evaluate()
{$
    Assert_c(isValid);
    if (!isValid)
    {
        errorCode = ERR_INVALID_TREE_STRUCTURE;
        $$$("Invalid structure");
        return;
    }

    errorCode = genFunctionTable();
    if (errorCode) {
        $$$("Problems with generating function table");
        return;
    }
    isCurrentFunctionCalledReturn = 0;
    if (functionsTable.count(entryPointHash))
        customFunctionEvaluate(entryPointHash, programStack);
    else{
        errorCode = ERR_NO_ENTRY_POINT_FUNCTION;
        $$$("There is no entry point. By default use main().");
        return;
    }
    $$ return;
}
//|_____________________________________________________________________________________________________________________________|
//===============================================================================================================================


//===============================================================================================================================
//|                                                                                                                             |
//|                                                                                                                             |
//|                                  это самая вызываемая функция из всех написанных                                            |
//|                                         (создает новый узел для дерева)                                                     |
//|                                                                                                                             |
//|_____________________________________________________________________________________________________________________________|
//|                                                                                                                             |
Expression::TNode* createNode(Expression::TNode* left, Expression::TNode* right, NodeType typeNode, UnionData unionData, Expression::TNode* parent)
{
    Expression::TNode* result = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
    result->ptrToData = (NodeInfo*)calloc(1, sizeof(NodeInfo));
    
    result->ptrToData->dataUnion = unionData;
    result->ptrToData->type = typeNode;
    result->link[0] = left;
    result->link[1] = right;
    result->parent = parent;
    if (result->link[0]) result->link[0]->parent = result;
    if (result->link[1]) result->link[1]->parent = result;

    return result;
}
//|_____________________________________________________________________________________________________________________________|
//===============================================================================================================================

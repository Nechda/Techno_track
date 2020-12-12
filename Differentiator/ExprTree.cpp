#include "ExprTree.h"
#include "LibStr.h"
#include "Logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <clocale>
#include "Parser.h"
#include "CallStack.h"



#define ACCURACY 1E-3
#define isZero(a) fabs((a)) < ACCURACY
#define isInteger(a) isZero( (a) - ceil(a) )
#define setLinkParent(node_)\
    if ( (node_)->link[0] ) (node_)->link[0]->parent = (node_);\
    if ( (node_)->link[1] ) (node_)->link[1]->parent = (node_);



bool Expression::isValidStruct()
{
    return isValid;
}

C_string expInfoToStr(NodeInfo* exp, bool printAddr = false)
{
    $
    if (!exp)
        return "";
    static char buffer[64] = {};
    static char* opDict = "+-*/^";
    ui8 offset = 0;
    bool isIntNumber = isInteger(exp->data.number);
    switch (exp->type)
    {
        case NODE_TYPE_NUMBER:
            if(isIntNumber)
                sprintf(buffer,"%d",(int)exp->data.number);
            else
                sprintf(buffer, "%.3lf", exp->data.number);
            break;
        case NODE_TYPE_OPERATION:
            sprintf(buffer, "%c", opDict[exp->data.opNumber]);
            break;
        case NODE_TYPE_VARIABLE:
            sprintf(buffer, "x");
            break;
        case NODE_TYPE_FUNCTION:
            sprintf(buffer, "%s", FUNCTION_NAMES_TABLE[exp->data.opNumber]);
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

Expression::Expression(const Expression& exp)
{$
    setRoot(rCopy<NodeInfo>(exp.ground.link[0]));
    $$
}

Expression::Expression(const C_string filename) : Tree(filename)
{$
    readTreeFromFile(filename);
    $$
};


//===============================================================================================================================
//|                                                                                                                             |
//|                                                                                                                             |
//|                           распечатка дерева в инфиксную запись + генерация dot файла                                        |
//|                                                                                                                             |
//|_____________________________________________________________________________________________________________________________|
//|                                                                                                                             |

/*
    \brief Функция генерации dot файла для graphviz
*/
void Expression::printNodeInDotFile(TNode* node, Stream stream)
{$
    Assert_c(stream);
    Assert_c(node);
    if (!stream || !node)
    {
        $$$("NULL ptr in node or stream structure");
        return;
    }

    bool isLeaf = !node->link[0] && !node->link[1];

    if (isLeaf)
    {
        fprintf(stream,
            "\"%s\" %s \n", expInfoToStr(node->ptrToData, true), DOT_LEAF_STYLE
        );
    }
    else
    {
        printNodeInDotFile(node->link[0], stream);
        printNodeInDotFile(node->link[1], stream);


        fprintf(stream, " \"%s\" \n", expInfoToStr(node->ptrToData,true));
        for (ui8 i = 0; i < 2; i++)
            if (node->link[i])
            {
                fprintf(stream, " \"%s\" -> ", expInfoToStr(node->ptrToData, true));
                fprintf(stream, " \"%s\" [label = \"L[%d]\", fontsize = 14] \n", expInfoToStr(node->link[i]->ptrToData, true), i);
            }
    }
    $$
}

/*
    \brief Реализация распечатки дерева в файл (инфиксная запись)
*/
static void printInteration(const Expression::TNode* root, int level, Stream stream)
{
    $
    Assert_c(stream);
    if (!stream)
    {
        $$$("stream is NULL");
        return;
    }
    if (!root)
    {
        $$
        return;
    }

    bool isLeaf = !root->link[0] && !root->link[1];

    if (isLeaf)
    {
        fprintf(stream, "(%s)", expInfoToStr(root->ptrToData));
        $$
        return;
    }
    else
    {
        if (level)
            fprintf(stream, "(");
        if (root->ptrToData->type == NODE_TYPE_FUNCTION)
        {
            fprintf(stream, "%s",FUNCTION_NAMES_TABLE[root->ptrToData->data.opNumber]);
            printInteration(root->link[0], level + 1, stream);
        }
        else
        {
            printInteration(root->link[0], level + 1, stream);
            fprintf(stream, "%s", expInfoToStr(root->ptrToData));
            printInteration(root->link[1], level + 1, stream);
        }
        if (level)
            fprintf(stream, ")");
        $$
        return;
    }
    $$
}

void Expression::writeTreeInFile(TNode* node, ui32 level, Stream stream)
{$
    printInteration(getRoot(), 0, stream);
    $$
}

/*
    \brief Реализация чтения дерева из файл (инфиксная запись)
*/
NodeInfo* getData(C_string str, ui32 len)
{$
    NodeInfo* ptr = (NodeInfo*)calloc(1, sizeof(NodeInfo));
    static char tmpStr[32] = {};
    memcpy(tmpStr, str, len);
    tmpStr[len] = 0;


    switch (tmpStr[0])
    {
    case '+':
        ptr->type = NODE_TYPE_OPERATION;
        ptr->data.opNumber = OP_SUM;
        break;
    case '-':
        ptr->type = NODE_TYPE_OPERATION;
        ptr->data.opNumber = OP_SUB;
        break;
    case '*':
        ptr->type = NODE_TYPE_OPERATION;
        ptr->data.opNumber = OP_MUL;
        break;
    case '/':
        ptr->type = NODE_TYPE_OPERATION;
        ptr->data.opNumber = OP_DIV;
        break;
    case '^':
        ptr->type = NODE_TYPE_OPERATION;
        ptr->data.opNumber = OP_POW;
        break;
    case 'x':
        ptr->type = NODE_TYPE_VARIABLE;
        ptr->data.opNumber = 0;
        break;
    default:
        ptr->type = NODE_TYPE_NUMBER;
        ptr->data.number = atof(tmpStr);
        break;
    }


    $$
    return ptr;
}

static void readInteration_Infix(Expression::TNode* node, C_string& str, Expression::TNode* TREEROOT)
{$
    ui8 linkIndex = 0;
    C_string c = NULL;
    while (str[0])
    {
        if (str[0] == '(')
        {
            node->link[linkIndex] = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
            node->link[linkIndex]->parent = node;
            str++;
            readInteration_Infix(node->link[linkIndex], str, TREEROOT);
            linkIndex++;
        }
        if (str[0] == ')')
        {
            str++;
            $$
            return;
        }
        if (str[0])
        {
            c = str;
            while (!strchr("()", *c))
                c++;
            node->ptrToData = getData(str, c - str);
            str = c;
            if (*c == ')')
            {
                str++;
                $$
                return;
            }
            else
                continue;
        }
    }
    $$
}

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

    errorCode = removeExtraChar(&buffer, "(+-*/^.)");
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


#define OP_DEFINE(symbol, name, enumName, priority, implentation,texPrintImplemenation)\
    static double runEvaluate##name(double left, double right) implentation
#define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)\
    static double runEvaluate##name(double left, double right) implentation
#include "FUNCTIONS.h"
#undef OP_DEFINE(symbol, name, enumName, priority, implentation,texPrintImplemenation)
#undef FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)



FuncionType evaluateFunctions[] = {
    #define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)
    #define OP_DEFINE(symbol, name, enumName, priority, implentation,texPrintImplemenation)\
        runEvaluate##name,
    #include "FUNCTIONS.h"
    #undef OP_DEFINE
    #undef FUNC_DEFINE
};

FuncionType evaluateStandartFunctions[] =
{
    #define OP_DEFINE(symbol, name, enumName, priority, implentation,texPrintImplemenation)
    #define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)\
        runEvaluate##name,
    #include "FUNCTIONS.h"
    #undef FUNC_DEFINE
    #undef OP_DEFINE
};

static bool constantSimplify(Expression::TNode* node, bool& isChangedTree)
{
    if (!node)
        return true;
    $
    bool isLeaf = !node->link[0] && !node->link[1];
    if (isLeaf)
    {
        $$
        return node->ptrToData->type == NODE_TYPE_NUMBER;
    }


    bool canEvaluate = 1;
    canEvaluate &= constantSimplify(node->link[0], isChangedTree);
    canEvaluate &= constantSimplify(node->link[1], isChangedTree);
    if (!canEvaluate) { $$; return false; }


    #define getNum(p) (p) ? (p)->ptrToData->data.number : 0
    double num[2] = {};
    num[0] = getNum(node->link[0]);
    num[1] = getNum(node->link[1]);
    #undef getNum

    FuncionType* evauateFunction = node->ptrToData->type == NODE_TYPE_OPERATION ? evaluateFunctions : evaluateStandartFunctions;
    node->ptrToData->data.number = evauateFunction[node->ptrToData->data.opNumber](num[0], num[1]);
    node->ptrToData->type = NODE_TYPE_NUMBER;

    rCleanUp(node->link[0]);
    rCleanUp(node->link[1]);
    node->link[0] = NULL;
    node->link[1] = NULL;

    isChangedTree |= 1;
    $$
    return true;
}

static void identitySimplify(Expression::TNode* node, bool& isChangedTree)
{
    if (!node)
        return;
    $
    bool isLeaf = !node->link[0] && !node->link[1];
    if (isLeaf) { $$;  return; }


    identitySimplify(node->link[0], isChangedTree);
    identitySimplify(node->link[1], isChangedTree);

    
    #define isZeroNumber(p) isZero(p->ptrToData->data.number - 0) && p->ptrToData->type == NODE_TYPE_NUMBER
    #define isOneNumber(p)  isZero(p->ptrToData->data.number - 1) && p->ptrToData->type == NODE_TYPE_NUMBER

    #define replaceLink(node_, oldLink_, newLink_)              \
        if((node_))                                             \
        for (ui8 i = 0; i < 2; i++)                             \
            if ((node_)->link[i] == (oldLink_) && (node_))      \
                (node_)->link[i] = (newLink_);                    

    if (node->ptrToData->type == NODE_TYPE_OPERATION)
    {
        Expression::TNode* parent = NULL;
        Expression::TNode* newLink = NULL;
        Expression::TNode* tmpLink = NULL;
        switch (node->ptrToData->data.opNumber)
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
                node->ptrToData->data.opNumber = FUNC_NEG;
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
                    newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, 0.0, parent);
                    //createNode(newLink, NODE_TYPE_NUMBER);
                    //newLink->ptrToData->data.number = 0;
                    //newLink->parent = parent;
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
                newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, 1.0, parent);
                //createNode(newLink, NODE_TYPE_NUMBER);
                //newLink->ptrToData->data.number = 1;
                //newLink->parent = parent;
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
                newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, 1.0, parent);
                //createNode(newLink, NODE_TYPE_NUMBER);
                //newLink->ptrToData->data.number = 1;
                //newLink->parent = parent;
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
                newLink = createNode(NULL, NULL, NODE_TYPE_NUMBER, 0.0, parent);
                //createNode(newLink, NODE_TYPE_NUMBER);
                //newLink->ptrToData->data.number = 0;
                //newLink->parent = parent;
                replaceLink(parent, node, newLink);

                rCleanUp(node);
                isChangedTree |= 1;
                $$
                    return;
            }


            // (E^a)^b = E^(a+b)
            if (node->link[0]->ptrToData->type == NODE_TYPE_OPERATION
            &&  node->link[0]->ptrToData->data.opNumber == OP_POW
                )
            {
                parent = node->parent;
                newLink = createNode(node->link[0]->link[1], node->link[1], NODE_TYPE_OPERATION, OP_SUM, node);
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
static double treeEvaluate(Expression::TNode* node,const double variable)
{
    if (!node)
        return 0;
    $
    bool isLeaf = !node->link[0] && !node->link[1];
    if (isLeaf)
    {
        $$
        return node->ptrToData->type == NODE_TYPE_NUMBER ? node->ptrToData->data.number : variable;
    }

    double num[2] = {};
    num[0] = treeEvaluate(node->link[0], variable);
    num[1] = treeEvaluate(node->link[1], variable);

    FuncionType* evauateFunction = node->ptrToData->type == NODE_TYPE_OPERATION ? evaluateFunctions : evaluateStandartFunctions;
    double result = evauateFunction[node->ptrToData->data.opNumber](num[0], num[1]);
    $$
    return result;
}

double Expression::evaluate(double variable)
{$
    Assert_c(isValid);
    if (!isValid)
    {
        $$$("Invalid structure");
        return 0;
    }
    $$
    return treeEvaluate(getRoot(), variable);
}
//|_____________________________________________________________________________________________________________________________|
//===============================================================================================================================


//===============================================================================================================================
//|                                                                                                                             |
//|                                                                                                                             |
//|                                                 генерация теха                                                              |
//|                                                                                                                             |
//|                                                                                                                             |
//|_____________________________________________________________________________________________________________________________|
//|                                                                                                                             |
static ui8 getPriority(const Expression::TNode* node)
{
    if (!node)
        return 3;
    NodeType type = node->ptrToData->type;
    if (type == NODE_TYPE_NUMBER || type == NODE_TYPE_VARIABLE || type == NODE_TYPE_FUNCTION)
        return 0;

    if (type == NODE_TYPE_OPERATION)
        switch (node->ptrToData->data.opNumber)
        {
            case OP_SUM:
            case OP_SUB:
                return 2;
            case OP_MUL:
            case OP_DIV:
            case OP_POW:
                return 1;
            default:
                return 3;
        }
}

static inline void printTex_OP_DIV(const Expression::TNode* root, int level, Stream stream);
static inline void printTex_OP_POW(const Expression::TNode* root, int level, Stream stream);
static inline void printTex_OP_MUL(const Expression::TNode* root, int level, Stream stream);
static inline void printTex_OP_STANDART(const Expression::TNode* root, int level, Stream stream);
static inline void printTex_FUNC_SQRT(const Expression::TNode* root, int level, Stream stream);
static inline void printTex_FUNC_NEG(const Expression::TNode* root, int level, Stream stream);
static inline void printTex_FUNC_STANDART(const Expression::TNode* root, int level, Stream stream);

void genTexInteration(const Expression::TNode* root, int level, Stream stream)
{$
    Assert_c(stream);
    if (!stream){ $$$("stream is NULL"); return; }
    if (!root) { $$; return; }

    bool isLeaf = !root->link[0] && !root->link[1];
    if (isLeaf)
    {
        fprintf(stream, "%s", expInfoToStr(root->ptrToData));
        $$
        return;
    }


    if(root->ptrToData->type == NODE_TYPE_OPERATION)
    switch (root->ptrToData->data.opNumber)
    {
        case OP_DIV:
            printTex_OP_DIV(root, level, stream); $$; return;
            break;
        case OP_POW:
            printTex_OP_POW(root, level, stream); $$; return;
            break;
        case OP_MUL:
            printTex_OP_MUL(root, level, stream); $$; return;
        default:
            printTex_OP_STANDART(root, level, stream); $$; return;
            break;
    }


    if(root->ptrToData->type == NODE_TYPE_FUNCTION)
    switch (root->ptrToData->data.opNumber)
    {
        case FUNC_SQRT:
            printTex_FUNC_SQRT(root, level, stream); $$; return;
            break;
        case FUNC_NEG:
            printTex_FUNC_NEG(root, level, stream); $$; return;
        default:
            printTex_FUNC_STANDART(root, level, stream); $$; return;
            break;
    } 
    $$
    return;
}

static inline void printTex_OP_DIV(const Expression::TNode* root, int level, Stream stream)
{
    fprintf(stream, "\\frac{");
    genTexInteration(root->link[0], level + 1, stream);
    fprintf(stream, "}{");
    genTexInteration(root->link[1], level + 1, stream);
    fprintf(stream, "}");
}

static inline void printTex_OP_POW(const Expression::TNode* root, int level, Stream stream)
{
    ui8 mainPriority = getPriority(root);
    ui8 linkPriority = getPriority(root->link[0]);
    if (linkPriority > mainPriority)
        fprintf(stream, "\\left(");
    genTexInteration(root->link[0], level + 1, stream);
    if (linkPriority > mainPriority)
        fprintf(stream, "\\right)");
    fprintf(stream, "^{");
    genTexInteration(root->link[1], level + 1, stream);
    fprintf(stream, "}");
}

static inline void printTex_OP_MUL(const Expression::TNode* root, int level, Stream stream)
{
    ui8 mainPriority = getPriority(root);
    ui8 linkPriority = 0;
    bool isNeg = 0;
    #define isNumberNode(p) ( (p) ? (p)->ptrToData->type == NODE_TYPE_NUMBER : 0)
    #define isDivOrMulNode(p) ( (p) ? (p)->ptrToData->type == NODE_TYPE_OPERATION && (p)->ptrToData->data.opNumber == OP_DIV || (p)->ptrToData->data.opNumber == OP_MUL : 0)

    bool useStar = isNumberNode(root->link[0]) && isNumberNode(root->link[1]);
    useStar |= isNumberNode(root->link[0]) && isDivOrMulNode(root->link[1]) || isNumberNode(root->link[1]) && isDivOrMulNode(root->link[0]);

    bool needInverseOrder = isNumberNode(root->link[1]);

    for (ui8 i = 0; i < 2; i++)
    {
        ui8 index = needInverseOrder ? 1-i : i;
        isNeg = root->link[index]->ptrToData->type == NODE_TYPE_FUNCTION
             && root->link[index]->ptrToData->data.opNumber == FUNC_NEG
             || root->link[index]->ptrToData->type == NODE_TYPE_NUMBER
             && root->link[index]->ptrToData->data.number < 0;
        linkPriority = getPriority(root->link[index]);
        if (linkPriority > mainPriority || isNeg)
            fprintf(stream, "\\left(");

        genTexInteration(root->link[index], level + 1, stream);
        if (linkPriority > mainPriority || isNeg)
            fprintf(stream, "\\right)");

        if (i < 1 && useStar)
            fprintf(stream, "\\cdot ");
    }
    #undef order
    #undef isNumberNode
    #undef isDivNode
}

static inline void printTex_OP_STANDART(const Expression::TNode* root, int level, Stream stream)
{
    ui8 mainPriority = getPriority(root);
    ui8 linkPriority = 0;
    for (ui8 i = 0; i < 2; i++)
    {
        linkPriority = getPriority(root->link[i]);
        if (linkPriority > mainPriority)
            fprintf(stream, "\\left(");

        genTexInteration(root->link[i], level + 1, stream);
        if (linkPriority > mainPriority)
            fprintf(stream, "\\right)");

        if (i < 1)
            fprintf(stream, "%s", expInfoToStr(root->ptrToData));
    }
}



static inline void printTex_FUNC_SQRT(const Expression::TNode* root, int level, Stream stream)
{
    fprintf(stream, "\\%s{", FUNCTION_NAMES_TABLE[root->ptrToData->data.opNumber]);
    genTexInteration(root->link[0], level + 1, stream);
    fprintf(stream, "}");
}

static inline void printTex_FUNC_NEG(const Expression::TNode* root, int level, Stream stream)
{
    ui8 mainPriority = getPriority(root);
    ui8 linkPriority = getPriority(root->link[0]);
    fprintf(stream, "-");
    if (linkPriority > mainPriority)
        fprintf(stream, "\\left(");
    genTexInteration(root->link[0], level + 1, stream);
    if (linkPriority > mainPriority)
        fprintf(stream, "\\right)");
}

static inline void printTex_FUNC_STANDART(const Expression::TNode* root, int level, Stream stream)
{
    fprintf(stream, "\\%s\\left(", FUNCTION_NAMES_TABLE[root->ptrToData->data.opNumber]);
    genTexInteration(root->link[0], level + 1, stream);
    fprintf(stream, "\\right)");
}

void Expression::genTex(Stream stream)
{$
    Assert_c(isValid);
    if (!isValid)
    {
        $$$("Invalid structure");
        return;
    }
    genTexInteration(getRoot(), 0, stream);
    if (stream == stdout)
        printf("\n");
    $$
}

void Expression::genTexFile(const C_string outFilename)
{$
    Assert_c(isValid);
    if (!isValid)
    {
        $$$("Invalid structure");
        return;
    }
    char buffer[256];
    Assert_c(outFilename);
    if (!outFilename)
    {
        $$$("NULL ptr in C_string outFilename")
        return;
    }

    sprintf(buffer, "%s.tex", outFilename);
    FILE* file = fopen(buffer, "w");
    Assert_c(file);
    if (!file)
    {
        $$$("NULL ptr in FILE* file");
        return;
    }

    fprintf(file,
        "\\documentclass[preview]{standalone}\n"
        "\\begin{document}\n"
        "$"
    );
    genTex(file);
    fprintf(file,
        "$\n"
        "\\end{document}\n"
    );
        

    fclose(file);

    sprintf(buffer, "pdflatex.exe -quiet -interaction=nonstopmode %s.tex", outFilename);
    system(buffer);
    $$
}



void Expression::getStepByStepSimplificationTex(const C_string outFilename, i16 derivativeOrder, ui8 paperType)
{$
    Assert_c(isValid);
    if (!isValid)
    {
        $$$("Invalid structure");
        return;
    }
    char buffer[256];
    Assert_c(outFilename);
    if (!outFilename)
    {
        $$$("NULL ptr in C_string outFilename")
        return;
    }

    sprintf(buffer, "%s.tex", outFilename);
    FILE* file = fopen(buffer, "w");
    Assert_c(file);
    if (!file)
    {
        $$$("NULL ptr in FILE* file");
        return;
    }

    fprintf(file,
        "\\documentclass[12pt]{article}\n"
        "\\usepackage[T1,T2A]{fontenc}\n"
        "\\usepackage[utf8x]{inputenc}\n"
        "\\usepackage[russian]{babel}\n"
        "\\usepackage[left=1cm,right=1cm,top=1cm,bottom=2cm,a%dpaper,landscape]{geometry}\n"
        "\\begin{document}\n",
        paperType
    );

    wchar_t* constantSimplifyPhrases[] = {
        L"Подсчитываем все, что можно подсчитать:",
        L"И еще раз собираем подобные слагаемые:"
    };

    wchar_t* identitySimplifyPhrases[] = {
        L"Чтож, теперь избавимся от тривиальных выражений",
        L"После упрощения тривиальных комбинаций получим:"
    };

    wchar_t* jokes[] = {
        L"На это моменте мне надоело техать кажду строчку, поэтому буду писать результаты преобразований.",
        L"Поэтому я пропущу несколько тривиальных преобразований.",
        L"А теперь представьте, что где-то я допустил ошибку..."
    };

    std::setlocale(LC_ALL, "en_US.utf8");

    ui8 indecies[2] = { 0,0 };
    ui8 jokeIndex = 0;
    ui32 iteration = 1;
    bool writeExplanation = 1;
    bool isChangedTree = 0;

    fprintf(file, "%ls\n", L"Возьмем производную функции:");
    fprintf(file, "\\[");
    genTex(file);
    fprintf(file, "\\]\n");
    fprintf(file, "%ls\n", 
        L"Тривиальные вычисления позволяют нам записать общий вид производной,"
        L"однако в таком виде писать её в лабе неприемлемо, поэтому нам необходимо "
        L"произвести ряд упрощений."
    );

    while (derivativeOrder)
    {
        differentiate();
        fprintf(file, "\\[");
        genTex(file);
        fprintf(file, "\\]\n");
        do
        {
            isChangedTree = 0;
            constantSimplify(getRoot(), isChangedTree);

            if (isChangedTree && iteration % 5 == 0)
            {
                fprintf(file, "%ls\n", jokes[jokeIndex]);
                writeExplanation = 1;
                if (jokeIndex == 0)
                    writeExplanation = 0;
                jokeIndex++;
                jokeIndex %= sizeof(jokes) / sizeof(jokes[0]);
            }

            if (isChangedTree && writeExplanation)
            {
                fprintf(file, "%ls\n", constantSimplifyPhrases[indecies[0]]);
                indecies[0]++;
                indecies[0] %= sizeof(constantSimplifyPhrases) / sizeof(constantSimplifyPhrases[0]);
                fprintf(file, "\\[");
                genTex(file);
                fprintf(file, "\\]\n");
                iteration++;
                continue;
            }
            identitySimplify(getRoot(), isChangedTree);
            if (isChangedTree && writeExplanation)
            {
                fprintf(file, "%ls\n", identitySimplifyPhrases[indecies[1]]);
                indecies[1]++;
                indecies[1] %= sizeof(identitySimplifyPhrases) / sizeof(identitySimplifyPhrases[0]);
                fprintf(file, "\\[");
                genTex(file);
                fprintf(file, "\\]\n");
                iteration++;
                continue;
            }
        } while (isChangedTree);

        fprintf(file, "%ls\n", L"Хватит упрощать!");
        fprintf(file, "\\[");
        genTex(file);
        fprintf(file, "\\]\n");
        derivativeOrder--;
        if(derivativeOrder)
            fprintf(file, "%ls\n", L"Продифференцируем еще раз:");
    }

    fprintf(file,
        "\\end{document}\n"
    );
        

    fclose(file);

    sprintf(buffer, "pdflatex.exe -quiet -interaction=nonstopmode %s.tex", outFilename);
    system(buffer);

    $$
}



//|_____________________________________________________________________________________________________________________________|
//===============================================================================================================================


//===============================================================================================================================
//|                                                                                                                             |
//|                                                                                                                             |
//|                                                 дифференцирование выражений                                                 |
//|                                                                                                                             |
//|                                                                                                                             |
//|_____________________________________________________________________________________________________________________________|
//|                                                                                                                             |
Expression::TNode* createNode(Expression::TNode* left, Expression::TNode* right, NodeType typeNode, double value, Expression::TNode* parent)
{
    Expression::TNode* result = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
    result->ptrToData = (NodeInfo*)calloc(1, sizeof(NodeInfo));
    
    if(typeNode == NODE_TYPE_NUMBER)
        result->ptrToData->data.number = value;
    else
        result->ptrToData->data.opNumber = static_cast<ui32>(value);

    result->ptrToData->type = typeNode;

    result->link[0] = left;
    result->link[1] = right;

    result->parent = parent;
    return result;
}


#define c(r) rCopy(r)
#define d(r) differentiateInteration(r)


static void differentiateInteration(Expression::TNode* root);

static void diff_MUL(Expression::TNode* node)
{
    Expression::TNode *f, *g, *df, *dg;
    f = node->link[0];
    g = node->link[1];
    df = c(f);
    dg = c(g);
    d(df);
    d(dg);

    node->ptrToData->data.opNumber = OP_SUM;
    
    node->link[0] = createNode(f, dg, NODE_TYPE_OPERATION, OP_MUL, node);
    node->link[1] = createNode(df, g, NODE_TYPE_OPERATION, OP_MUL, node);

    setLinkParent(node);
    setLinkParent(node->link[0]);
    setLinkParent(node->link[1]);
}

static void diff_DIV(Expression::TNode* node)
{
    Expression::TNode *f, *g, *df, *dg, *g2;
    f = node->link[0];
    g = node->link[1];
    g2 = c(g);
    df = c(f);
    dg = c(g);
    d(df);
    d(dg);

    node->link[0] = createNode(
        createNode(df,g, NODE_TYPE_OPERATION, OP_MUL,NULL),
        createNode(f, dg, NODE_TYPE_OPERATION, OP_MUL, NULL),
        NODE_TYPE_OPERATION, OP_SUB, node);

    node->link[1] = createNode(
        g2,
        createNode(NULL, NULL, NODE_TYPE_NUMBER, 2.0, NULL),
        NODE_TYPE_OPERATION, OP_POW, node);

    Expression::TNode* up = node->link[0];
    setLinkParent(up);
    setLinkParent(up->link[0]);
    setLinkParent(up->link[1]);

    Expression::TNode* down = node->link[1];
    setLinkParent(down);
}

static void diff_POW(Expression::TNode* node)
{
    NodeType lType = node->link[0]->ptrToData->type;
    NodeType rType = node->link[1]->ptrToData->type;

    Expression::TNode *f, *g, *y;

    if (lType == NODE_TYPE_VARIABLE && rType == NODE_TYPE_NUMBER)
    {
        //тупо степенная функция
        y = c(node->link[1]);
        node->link[1]->ptrToData->data.number--;

        node->link[1] = c(node);
        node->ptrToData->data.opNumber = OP_MUL;
        node->link[0] = y;

        setLinkParent(node);
        setLinkParent(node->link[1]);
        return;
    }

    if (lType == NODE_TYPE_NUMBER && rType == NODE_TYPE_VARIABLE)
    {
        node->link[1] = c(node);
        node->link[0] = createNode(node->link[0], NULL, NODE_TYPE_FUNCTION, FUNC_LN, node); // делаем логарифм
        node->ptrToData->data.opNumber = OP_MUL;

        setLinkParent(node);
        setLinkParent(node->link[1]);
        return;
    }

    //y = f^g -> y' = y (g ln f)'

    y = c(node);
    f = c(node->link[0]);
    g = c(node->link[1]);
    
    rCleanUp(node->link[0]);
    rCleanUp(node->link[1]);

    node->ptrToData->data.opNumber = OP_MUL;
    node->link[0] = y;
    node->link[1] = createNode(
        g,
        createNode(
            f,
            NULL,
            NODE_TYPE_FUNCTION,
            FUNC_LN,
            NULL
        ),
        NODE_TYPE_OPERATION, OP_MUL, NULL);

    setLinkParent(node);
    setLinkParent(node->link[1]);

    Expression::TNode* lnNode = node->link[1]->link[1];
    lnNode->link[0]->parent = lnNode;

    d(node->link[1]);
}


#define OP_DEFINE(symbol, name, enumName, priority, implentation, diffImplementation) 
#define FUNC_DEFINE(name, enumName, implentation, diffImplementation)\
    static void diff_##name(Expression::TNode* node) diffImplementation
#include "FUNCTIONS.h"
#undef OP_DEFINE
#undef FUNC_DEFINE

static void differentiateInteration(Expression::TNode* root)
{$
    if (!root) { $$; return; }                

    NodeInfo& nodeInfo = *root->ptrToData;
    if (nodeInfo.type == NODE_TYPE_NUMBER)
    {
        nodeInfo.data.number = 0;
        return;
    }

    if (nodeInfo.type == NODE_TYPE_VARIABLE)
    {
        nodeInfo.type = NODE_TYPE_NUMBER;
        nodeInfo.data.number = 1;
        return;
    }

    Expression::TNode* newNode = NULL;

    if (nodeInfo.type == NODE_TYPE_OPERATION)
    switch (nodeInfo.data.opNumber)
    {
        case OP_SUM:
        case OP_SUB:
            d(root->link[0]);
            d(root->link[1]);
            $$; return;
        case OP_MUL:
            diff_MUL(root);
            $$; return;
        case OP_DIV:
            diff_DIV(root);
            $$; return;
        case OP_POW:
            diff_POW(root);
            $$; return;
        default:
            break;
    }


    if (nodeInfo.type == NODE_TYPE_FUNCTION)
        switch (nodeInfo.data.opNumber)
        {
            #define OP_DEFINE(symbol, name, enumName, priority, implentation,texPrintImplemenation)
            #define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)\
                case enumName:\
                    diff_##name(root);\
                    break;
            #include "FUNCTIONS.h"
            #undef OP_DEFINE
            #undef FUNC_DEFINE
        default:
            printf("____________Undefined function__________\n");
            $$$("We try to diff undefined function");
            return;
        }
    $$
}

void Expression::differentiate()
{$
    Assert_c(isValid);
    if (!isValid)
    {
        $$$("Invalid structure");
        return;
    }
    simplify();
    differentiateInteration(getRoot());
    $$
}
//|_____________________________________________________________________________________________________________________________|
//===============================================================================================================================

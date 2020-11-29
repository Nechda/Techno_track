#include "ExprTree.h"
#include "LibStr.h"
#include "Logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Parser.h"
#include "CallStack.h"



#define ACCURACY 1E-3
#define isZero(a) fabs((a)) < ACCURACY
#define isInteger(a) isZero( (a) - ceil(a) )



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


void printInteration(const Expression::TNode* root, int level, Stream stream)
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
!!! эта функция не умеет парсить функции sin,cos, ... !!!
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

void readInteration(Expression::TNode* node, C_string& str, Expression::TNode* TREEROOT)
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
            readInteration(node->link[linkIndex], str, TREEROOT);
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
    if (!filename || !isValid)
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

    errorCode = removeExtraChar(&buffer, "(+-*/)");
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(getRoot());
        setRoot(NULL);
        $$$("removeExtraChar has returned error");
        return;
    }

    C_string str = buffer;
    readInteration(getRoot(), str, getRoot());
    $$
}

typedef double(*FuncionType)(double, double);


#define OP_DEFINE(symbol, name, enumName, priority, implentation,texPrintImplemenation)\
    double runEvaluate##name(double left, double right) implentation
#define FUNC_DEFINE(name, enumName, implentation, texPrintImplemenation)\
    double runEvaluate##name(double left, double right) implentation
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
        for (ui8 i = 0; i < 2; i++)                             \
            if ((node_)->link[i] == (oldLink_) && (node_))      \
                (node_)->link[i] = (newLink_);                    

    if (node->ptrToData->type == NODE_TYPE_OPERATION)
    {
        Expression::TNode* parent = NULL;
        Expression::TNode* newLink = NULL;
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
        //case OP_SUB:
        //    break;
        case OP_MUL:

            // x * 0 = 0 * x = 0
            for (int j = 0; j < 2; j++)
                if (isZeroNumber(node->link[j]))
                {
                    parent = node->parent;
                    createNode(newLink, NODE_TYPE_NUMBER);
                    newLink->ptrToData->data.number = 0;
                    newLink->parent = parent;
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
                createNode(newLink, NODE_TYPE_NUMBER);
                newLink->ptrToData->data.number = 1;
                newLink->parent = parent;
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
                createNode(newLink, NODE_TYPE_NUMBER);
                newLink->ptrToData->data.number = 1;
                newLink->parent = parent;
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
                createNode(newLink, NODE_TYPE_NUMBER);
                newLink->ptrToData->data.number = 0;
                newLink->parent = parent;
                replaceLink(parent, node, newLink);

                rCleanUp(node);
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
    bool isChangedTree = 0;
    do
    {
        isChangedTree = 0;
        constantSimplify(getRoot(), isChangedTree);
        identitySimplify(getRoot(), isChangedTree);
    } while (isChangedTree);
    $$
}

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
{
    return treeEvaluate(getRoot(), variable);
}

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
    for (ui8 i = 0; i < 2; i++)
    {
        isNeg = root->link[i]->ptrToData->type == NODE_TYPE_FUNCTION 
             && root->link[i]->ptrToData->data.opNumber == FUNC_NEG 
             || root->link[i]->ptrToData->type == NODE_TYPE_NUMBER
             && root->link[i]->ptrToData->data.number < 0;
        linkPriority = getPriority(root->link[i]);
        if (linkPriority > mainPriority || isNeg)
            fprintf(stream, "\\left(");

        genTexInteration(root->link[i], level + 1, stream);
        if (linkPriority > mainPriority || isNeg)
            fprintf(stream, "\\right)");

        if (i < 1)
            fprintf(stream, "\\cdot ");
    }
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
    genTexInteration(getRoot(), 0, stream);
    if (stream == stdout)
        printf("\n");
    $$
}


static void differentiateInteration(Expression::TNode* root)
{
    if (!root)
        return;

    #define replaceLink(node_, oldLink_, newLink_)              \
        for (ui8 i = 0; i < 2; i++)                             \
            if ((node_)->link[i] == (oldLink_) && (node_))      \
                (node_)->link[i] = (newLink_);                    

    NodeInfo nodeInfo = *root->ptrToData;
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

    Expression::TNode* newLink[2] = { NULL, NULL };

    if (nodeInfo.type == NODE_TYPE_OPERATION)
        switch (nodeInfo.data.opNumber)
        {
        case OP_SUM:
        case OP_SUB:
            differentiateInteration(root->link[0]);
            differentiateInteration(root->link[1]);
            break;
        case OP_MUL:
            newLink[0] = rCopy(root->link[0]);
            newLink[1] = rCopy(root->link[1]);
            break;
        }

}

void Expression::differentiate()
{
    differentiateInteration(getRoot());
}
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
    static char* opDict = "+-*/";
    ui8 offset = 0;
    bool isIntNumber = isInteger(exp->data.number);
    switch (exp->type)
    {
        case EXP_NUMBER:
            if(isIntNumber)
                sprintf(buffer,"%d",(int)exp->data.number);
            else
                sprintf(buffer, "%.3lf", exp->data.number);
            break;
        case EXP_OPERATION:
            sprintf(buffer, "%c", opDict[exp->data.opNumber]);
            break;
        case EXP_VARIABLE:
            sprintf(buffer, "x");
            break;
        case EXP_FUNCTION:
            sprintf(buffer, "%s", function_names[exp->data.opNumber]);
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
            "\"%s\" %s \n", expInfoToStr(node->data, true), DOT_LEAF_STYLE
        );
    }
    else
    {
        printNodeInDotFile(node->link[0], stream);
        printNodeInDotFile(node->link[1], stream);


        fprintf(stream, " \"%s\" \n", expInfoToStr(node->data,true));
        for (ui8 i = 0; i < 2; i++)
            if (node->link[i])
            {
                fprintf(stream, " \"%s\" -> ", expInfoToStr(node->data, true));
                fprintf(stream, " \"%s\" [label = \"L[%d]\", fontsize = 14] \n", expInfoToStr(node->link[i]->data, true), i);
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
        fprintf(stream, "(%s)", expInfoToStr(root->data));
        $$
        return;
    }
    else
    {
        if (level)
            fprintf(stream, "(");
        if (root->data->type == EXP_FUNCTION)
        {
            fprintf(stream, "%s",function_names[root->data->data.opNumber]);
            printInteration(root->link[0], level + 1, stream);
        }
        else
        {
            printInteration(root->link[0], level + 1, stream);
            fprintf(stream, "%s", expInfoToStr(root->data));
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


NodeInfo* getData(C_string str, ui32 len)
{$
    NodeInfo* ptr = (NodeInfo*)calloc(1, sizeof(NodeInfo));
    static char tmpStr[32] = {};
    memcpy(tmpStr, str, len);
    tmpStr[len] = 0;


    switch (tmpStr[0])
    {
    case '+':
        ptr->type = EXP_OPERATION;
        ptr->data.opNumber = 0;
        break;
    case '-':
        ptr->type = EXP_OPERATION;
        ptr->data.opNumber = 1;
        break;
    case '*':
        ptr->type = EXP_OPERATION;
        ptr->data.opNumber = 2;
        break;
    case '/':
    case '\\':
        ptr->type = EXP_OPERATION;
        ptr->data.opNumber = 3;
        break;
    case 'x':
        ptr->type = EXP_VARIABLE;
        ptr->data.opNumber = 3;
        break;
    default:
        ptr->type = EXP_NUMBER;
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
            node->data = getData(str, c - str);
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

typedef double(*FunctionType)(double, double);


double runEvaluateSUM(double left, double right)
{
    return left + right;
}
double runEvaluateSUB(double left, double right)
{
    return left - right;
}
double runEvaluateMUL(double left, double right)
{
    return left * right;
}
double runEvaluateDIV(double left, double right)
{
    return left / right;
}

double runEvaluateSIN(double left, double right)
{
    return sinf(left);
}
double runEvaluateCOS(double left, double right)
{
    return cosf(left);
}
double runEvaluateTAN(double left, double right)
{
    return tanf(left);
}
double runEvaluateCOT(double left, double right)
{
    return 1.0 / tanf(left);
}
double runEvaluateLN(double left, double right)
{
    return log(left);
}

FunctionType evaluateFunctions[] = {
    runEvaluateSUM, runEvaluateSUB, runEvaluateMUL, runEvaluateDIV
};

FunctionType evaluateStandartFunctions[] =
{
    runEvaluateSIN, runEvaluateCOS, runEvaluateTAN, runEvaluateCOT, runEvaluateLN
};

bool constantSimplify(Expression::TNode* node, bool& isChangedTree)
{
    if (!node)
        return false;
    $
    bool isLeaf = !node->link[0] && !node->link[1];
    if (isLeaf)
    {
        $$
        return node->data->type == EXP_NUMBER ? 1 : 0;
    }



    if (node->data->type == EXP_OPERATION)
    {
        bool canEvaluate = 1;
        canEvaluate &= constantSimplify(node->link[0], isChangedTree);
        canEvaluate &= constantSimplify(node->link[1], isChangedTree);
        if (!canEvaluate)
        {
            $$
            return false;
        }

        double num[2] = {};
        num[0] = node->link[0]->data->data.number;
        num[1] = node->link[1]->data->data.number;

        node->data->data.number = evaluateFunctions[node->data->data.opNumber](num[0], num[1]);
        node->data->type = EXP_NUMBER;

        rCleanUp(node->link[0]);
        rCleanUp(node->link[1]);
        node->link[0] = NULL;
        node->link[1] = NULL;

        isChangedTree |= 1;

    }


    if (node->data->type == EXP_FUNCTION)
    {
        bool canEvaluate = 1;
        canEvaluate &= constantSimplify(node->link[0], isChangedTree);
        if (!canEvaluate)
        {
            $$
            return false;
        }

        double num[2] = {};
        num[0] = node->link[0]->data->data.number;
        num[1] = 0;

        node->data->data.number = evaluateStandartFunctions[node->data->data.opNumber](num[0], num[1]);
        node->data->type = EXP_NUMBER;

        rCleanUp(node->link[0]);
        rCleanUp(node->link[1]);
        node->link[0] = NULL;
        node->link[1] = NULL;

        isChangedTree |= 1;

    }




    $$
    return true;
}

void identitySimplify(Expression::TNode* node, bool& isChangedTree)
{
    if (!node)
        return;
    $
    bool isLeaf = !node->link[0] && !node->link[1];
    if (isLeaf)
    {
        $$
        return;
    }

    identitySimplify(node->link[0], isChangedTree);
    identitySimplify(node->link[1], isChangedTree);

    
    #define isZeroNumber(p) isZero(p->data->data.number) && p->data->type == EXP_NUMBER
    #define isOneNumber(p) isZero(p->data->data.number-1) && p->data->type == EXP_NUMBER

    if (node->data->type == EXP_OPERATION)
    {
        Expression::TNode* prnt = NULL;
        Expression::TNode* lnk = NULL;
        switch (node->data->data.opNumber)
        {
        case 0:
            for (int j = 0; j<2; j++)
                if (isZeroNumber(node->link[j]))
                {
                    prnt = node->parent;
                    lnk = node->link[j ^ 1];

                    for (ui8 i = 0; i < 2; i++)
                        if (prnt->link[i] == node && prnt)
                            prnt->link[i] = lnk;

                    lnk->parent = prnt;

                    rCleanUp(node->link[j]);
                    free(node->data);
                    free(node);
                    isChangedTree |= 1;
                    $$
                    return;
                }
            break;
            //case 1:
            //    break;
        case 2:

            for (int j = 0; j<2; j++)
                if (isZeroNumber(node->link[j]))
                {
                    prnt = node->parent;

                    lnk = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
                    lnk->parent = prnt;
                    lnk->data = (NodeInfo*)calloc(1, sizeof(NodeInfo));
                    lnk->data->type = EXP_NUMBER;
                    lnk->data->data.number = 0;

                    for (ui8 i = 0; i < 2; i++)
                        if (prnt->link[i] == node && prnt)
                            prnt->link[i] = lnk;

                    rCleanUp(node);
                    isChangedTree |= 1;
                    $$
                    return;
                }


            for (int j = 0; j<2; j++)
                if (isOneNumber(node->link[j]))
                {
                    prnt = node->parent;
                    lnk = node->link[j ^ 1];

                    for (ui8 i = 0; i < 2; i++)
                        if (prnt->link[i] == node && prnt)
                            prnt->link[i] = lnk;

                    lnk->parent = prnt;

                    rCleanUp(node->link[j]);
                    free(node->data);
                    free(node);
                    isChangedTree |= 1;
                    $$
                    return;
                }
            break;
        case 3:

            if (isOneNumber(node->link[1]))
            {
                prnt = node->parent;
                lnk = node->link[0];

                for (ui8 i = 0; i < 2; i++)
                    if (prnt->link[i] == node && prnt)
                        prnt->link[i] = lnk;

                lnk->parent = prnt;

                rCleanUp(node->link[1]);
                free(node->data);
                free(node);
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


static ui8 getPriority(const Expression::TNode* node)
{
    if (!node)
        return 3;
    NodeType type = node->data->type;
    if (type == EXP_NUMBER || type == EXP_VARIABLE || type == EXP_FUNCTION)
        return 0;

    if (type == EXP_OPERATION)
        switch (node->data->data.opNumber)
        {
            case OP_SUM:
            case OP_SUB:
                return 2;
            case OP_MUL:
            case OP_DIV:
                return 1;
            default:
                return 3;
        }
}


void genTexInteration(const Expression::TNode* root, int level, Stream stream)
{$
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
        fprintf(stream, "%s", expInfoToStr(root->data));
        $$
        return;
    }
    else
    {

        if (root->data->data.opNumber == OP_DIV && root->data->type == EXP_OPERATION)
        {
            fprintf(stream, "\\frac{");
            genTexInteration(root->link[0], level + 1, stream);
            fprintf(stream, "}{");
            genTexInteration(root->link[1], level + 1, stream);
            fprintf(stream, "}");
            $$
            return;
        }

        if (root->data->type == EXP_FUNCTION)
        {
            fprintf(stream, "\\%s\\left(", function_names[root->data->data.opNumber]);
            genTexInteration(root->link[0], level + 1, stream);
            fprintf(stream, "\\right)");
            $$
            return;
        }

        ui8 mainPriority = getPriority(root);
        ui8 linkPriority = 0;
        for (ui8 i = 0; i < 2; i++)
        {
            ui8 linkPriority = getPriority(root->link[i]);
            if (linkPriority > mainPriority )
                fprintf(stream,"\\left(");
            
            genTexInteration(root->link[i], level + 1, stream);
            if (linkPriority > mainPriority)
                fprintf(stream, "\\right)");

            if (i < 1)
                fprintf(stream, "%s", expInfoToStr(root->data));
        }
        
        $$
        return;
    }
    $$
}

void Expression::genTex(Stream stream)
{$
    genTexInteration(getRoot(), 0, stream);
    if (stream == stdout)
        printf("\n");
    $$
}
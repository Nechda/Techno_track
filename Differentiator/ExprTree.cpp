#include "ExprTree.h"
#include "LibStr.h"
#include "Logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Parser.h"



C_string expInfoToStr(ExpInfo* exp)
{
    static char buffer[64] = {};
    static char* opDict = "+-*/";
    if (exp->expType == EXP_NUMBER)
    {
        sprintf(buffer, "%d\n0x%X", exp->value, exp);
        return buffer;
    }

    if (exp->expType == EXP_OPERATION)
    {
        sprintf(buffer, "%c\n0x%X", opDict[exp->value], exp);
        return buffer;
    }

    if (exp->expType == EXP_VARIABLE)
    {
        sprintf(buffer, "x\n0x%X", exp);
        return buffer;
    }

    return buffer;
}

Expression::Expression(const Expression& exp)
{
    setRoot(rCopy<ExpInfo>(exp.ground.link[0]));
}

void Expression::printNodeInDotFile(TNode* node, Stream stream)
{
    Assert_c(stream);
    Assert_c(node);
    if (!stream || !node)
        return;

    bool isLeaf = !node->link[0] && !node->link[1];

    if (isLeaf)
    {
        fprintf(stream,
            "\"%s\" %s \n", expInfoToStr(node->data), DOT_LEAF_STYLE
        );
    }
    else
    {
        printNodeInDotFile(node->link[0], stream);
        printNodeInDotFile(node->link[1], stream);

        fprintf(stream, " \"%s\" \n", expInfoToStr(node->data));

        fprintf(stream, " \"%s\" -> ", expInfoToStr(node->data));
        fprintf(stream, " \"%s\" [label = \"L[1]\", fontsize = 14] \n", expInfoToStr(node->link[1]->data));
        fprintf(stream, " \"%s\" -> ", expInfoToStr(node->data));
        fprintf(stream, " \"%s\" [label = \"L[0]\", fontsize = 14] \n", expInfoToStr(node->link[0]->data));
    }
}


void printInteration(const Expression::TNode* root, int level, Stream stream)
{
    Assert_c(stream);
    if (!stream)
        return;
    if (!root)
        return;

    bool isLeaf = !root->link[0] && !root->link[1];

    if (isLeaf)
    {
        fprintf(stream, "(%s)", expInfoToStr(root->data));
        return;
    }
    else
    {
        if (level)
            fprintf(stream, "(");
        printInteration(root->link[0], level + 1, stream);
        fprintf(stream, "%s", expInfoToStr(root->data));
        printInteration(root->link[1], level + 1, stream);
        if (level)
            fprintf(stream, ")");
        return;
    }
}

void Expression::writeTreeInFile(TNode* node, ui32 level, Stream stream)
{
    printInteration(getRoot(), 0, stream);
}


ExpInfo* getData(C_string str, ui32 len)
{
    ExpInfo* ptr = (ExpInfo*)calloc(1, sizeof(ExpInfo));
    static char tmpStr[32] = {};
    memcpy(tmpStr, str, len);
    tmpStr[len] = 0;


    switch (tmpStr[0])
    {
    case '+':
        ptr->expType = EXP_OPERATION;
        ptr->value = 0;
        break;
    case '-':
        ptr->expType = EXP_OPERATION;
        ptr->value = 1;
        break;
    case '*':
        ptr->expType = EXP_OPERATION;
        ptr->value = 2;
        break;
    case '/':
    case '\\':
        ptr->expType = EXP_OPERATION;
        ptr->value = 3;
        break;
    case 'x':
        ptr->expType = EXP_VARIABLE;
        ptr->value = 3;
        break;
    default:
        ptr->expType = EXP_NUMBER;
        ptr->value = atoi(tmpStr);
        break;
    }



    return ptr;
}

void readInteration(Expression::TNode* node, C_string& str, Expression::TNode* TREEROOT)
{
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
                return;
            }
            else
                continue;
        }
    }
}

void Expression::readTreeFromFile(const C_string filename)
{
    Assert_c(filename);
    if (!filename || !isValid)
        return;

    char* buffer = NULL;
    int errorCode = 0;
    errorCode = readFullFile(filename, &buffer);
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(getRoot());
        setRoot(NULL);
        return;
    }

    errorCode = removeExtraChar(&buffer, "(+-*/)");
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(getRoot());
        setRoot(NULL);
        return;
    }

    C_string str = buffer;
    readInteration(getRoot(), str, getRoot());

}

typedef ui32(*FunctionType)(ui32, ui32);


ui32 runEvaluateSUM(ui32 left, ui32 right)
{
    return left + right;
}
ui32 runEvaluateSUB(ui32 left, ui32 right)
{
    return left - right;
}
ui32 runEvaluateMUL(ui32 left, ui32 right)
{
    return left * right;
}
ui32 runEvaluateDIV(ui32 left, ui32 right)
{
    return left / right;
}


FunctionType evaluateFunctions[] = {
    runEvaluateSUM, runEvaluateSUB, runEvaluateMUL, runEvaluateDIV
};

bool constantSimplify(Expression::TNode* node, bool& isChangedTree)
{
    bool isLeaf = !node->link[0] && !node->link[1];
    if (isLeaf)
        return node->data->expType == EXP_NUMBER ? 1 : 0;



    if (node->data->expType == EXP_OPERATION)
    {
        bool canEvaluate = 1;
        canEvaluate &= constantSimplify(node->link[0], isChangedTree);
        canEvaluate &= constantSimplify(node->link[1], isChangedTree);
        if (!canEvaluate)
            return false;

        ui32 num[2] = {};
        num[0] = node->link[0]->data->value;
        num[1] = node->link[1]->data->value;

        node->data->value = evaluateFunctions[node->data->value](num[0], num[1]);
        node->data->expType = EXP_NUMBER;

        rCleanUp(node->link[0]);
        rCleanUp(node->link[1]);
        node->link[0] = NULL;
        node->link[1] = NULL;

        isChangedTree |= 1;
    }

    return true;
}

void identitySimplify(Expression::TNode* node, bool& isChangedTree)
{
    bool isLeaf = !node->link[0] && !node->link[1];
    if (isLeaf)
        return;

    identitySimplify(node->link[0], isChangedTree);
    identitySimplify(node->link[1], isChangedTree);


    #define isZeroNumber(p) p->data->value == 0 && p->data->expType == EXP_NUMBER
    #define isOneNumber(p) p->data->value == 1 && p->data->expType == EXP_NUMBER

    if (node->data->expType == EXP_OPERATION)
    {
        Expression::TNode* prnt = NULL;
        Expression::TNode* lnk = NULL;
        switch (node->data->value)
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
                    lnk->data = (ExpInfo*)calloc(1, sizeof(ExpInfo));
                    lnk->data->expType = EXP_NUMBER;
                    lnk->data->value = 0;

                    for (ui8 i = 0; i < 2; i++)
                        if (prnt->link[i] == node && prnt)
                            prnt->link[i] = lnk;

                    rCleanUp(node);
                    isChangedTree |= 1;
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
                return;
            }


            break;
        default:
            break;
        }
    }
    #undef isZeroNumber(p)
    #undef isOneNumber(p)
}

void Expression::simplify()
{
    bool isChangedTree = 0;
    do
    {
        isChangedTree = 0;
        constantSimplify(getRoot(), isChangedTree);
        identitySimplify(getRoot(), isChangedTree);
    } while (isChangedTree);
}

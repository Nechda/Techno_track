#include "Parser.h"
#include "CallStack.h"

Parser::~Parser()
{$

    tokens.clear();
$$
}

/*
    E -> E + T | E - T | T
    T -> T * F | T / F | F
    F -> N     | (E)
*/

ui32 Parser::isOperation(ui8 opType, ui32 start, ui32 end)
{$
    ui32 p = end;
    ui32 brackets = 0;
    while (start <= p && p<= end)
    {

        if (tokens[p].data.symbol == ')')
            do
            {
                if (tokens[p].data.symbol == ')') brackets++;
                if (tokens[p].data.symbol == '(') brackets--;
                p--;
            } while (brackets && p<end && p >start);

        if (p > end || p < start) break;

        if (tokens[p].type != LEX_OPERATION)
        {
            p--;
            continue;
        }
        if (tokens[p].data.symbol == opType)
        {
            $$
            return p;
        }
        p--;
    }
    $$
    return -1;
}

void Parser::parse_expr(Expression::TNode** ptrNode,ui32 start, ui32 end)
{$
    ui32 opPos = 0;
    static const ui8 operations[2] = { '+', '-' };

    for (ui8 i = 0; i < 2; i++)
    {
        opPos = isOperation(operations[i], start, end);
        if (opPos != -1)
        {
            *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
            (*ptrNode)->data = (ExpInfo*)calloc(1, sizeof(ExpInfo));
            (*ptrNode)->data->expType = EXP_OPERATION;
            (*ptrNode)->data->value = OP_SUM + i;

            parse_expr(&(*ptrNode)->link[0], start, opPos - 1);
            parse_term(&(*ptrNode)->link[1], opPos + 1, end  );
            $$
            return;
        }
    }

    parse_term(ptrNode, start, end);
    $$
}

void Parser::parse_term(Expression::TNode** ptrNode, ui32 start, ui32 end)
{$
    ui32 opPos = 0;
    static const ui8 operations[2] = { '*', '/' };

    for (ui8 i = 0; i < 2; i++)
    {
        opPos = isOperation(operations[i], start, end);
        if (opPos != -1)
        {
            *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
            (*ptrNode)->data = (ExpInfo*)calloc(1, sizeof(ExpInfo));
            (*ptrNode)->data->expType = EXP_OPERATION;
            (*ptrNode)->data->value = OP_MUL + i;

            parse_term(&(*ptrNode)->link[0], start, opPos - 1);
            parse_fact(&(*ptrNode)->link[1], opPos + 1, end);
            $$
            return;
        }
    }

    parse_fact(ptrNode, start, end);
   $$
}

void Parser::parse_fact(Expression::TNode** ptrNode, ui32 start, ui32 end)
{$
    ui32 brackets = 0;
    ui32 p = start;
    switch (tokens[start].data.symbol)
    {
    case '(':
        do
        {
            if (tokens[p].data.symbol == '(') brackets++;
            if (tokens[p].data.symbol == ')') brackets--;
            p++;
        } while (brackets && p<end);
        p--;
        parse_expr(ptrNode, start + 1, p - 1);
        break;
    case 'x':
        *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
        (*ptrNode)->data = (ExpInfo*)calloc(1, sizeof(ExpInfo));
        (*ptrNode)->data->expType = EXP_VARIABLE;
        (*ptrNode)->data->value = 0;
        break;
    default:
        *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
        (*ptrNode)->data = (ExpInfo*)calloc(1, sizeof(ExpInfo));
        (*ptrNode)->data->expType = EXP_NUMBER;
        (*ptrNode)->data->value = tokens[start].data.ivalue;
        break;
    }
    $$
}

Parser::Token Parser::getNextToken(C_string& str)
{
    $
    Token result;
    if (strchr("()", str[0]))
    {
        result.type = LEX_BRACKET;
        result.data.symbol = str[0];
        str++;
        $$
        return result;
    }

    if (str[0] == 'x')
    {
        result.type = LEX_VARIABLE;
        result.data.symbol = str[0];
        str++;
        $$
        return result;
    }

    if (strchr("+-*/\\", str[0]))
    {
        result.type = LEX_OPERATION;
        result.data.symbol = str[0];
        str++;
        $$
        return result;
    }

    ui32 offset = 0;
    sscanf(str, "%d%n", &result.data.ivalue, &offset);
    str += offset;
    result.type = LEX_NUMBER;
    $$
    return result;
}

Expression::TNode* Parser::parse(C_string expression)
{
    $
    Expression::TNode* root = NULL;
    tokens.clear();

    while (*expression)
        tokens.push_back(getNextToken(expression));

    #ifndef NDEBUG
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        printf("Token type: %d ");
        if (it->type == LEX_NUMBER)
            printf("data: %d\n", it->data.ivalue);
        else
            printf("data: %c\n", it->data.symbol);
    }
    #endif

    parse_expr(&root, 0, tokens.size() - 1);

    $$
    return root;
}

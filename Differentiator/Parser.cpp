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
            (*ptrNode)->data = (NodeInfo*)calloc(1, sizeof(NodeInfo));
            (*ptrNode)->data->type = EXP_OPERATION;
            (*ptrNode)->data->data.opNumber = OP_SUM + i;

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
            (*ptrNode)->data = (NodeInfo*)calloc(1, sizeof(NodeInfo));
            (*ptrNode)->data->type = EXP_OPERATION;
            (*ptrNode)->data->data.opNumber = OP_MUL + i;

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
    switch (tokens[start].type)
    {
    case LEX_BRACKET:
        do
        {
            if (tokens[p].data.symbol == '(') brackets++;
            if (tokens[p].data.symbol == ')') brackets--;
            p++;
        } while (brackets && p<end);
        p--;
        parse_expr(ptrNode, start + 1, p - 1);
        break;
    case LEX_VARIABLE:
        *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
        (*ptrNode)->data = (NodeInfo*)calloc(1, sizeof(NodeInfo));
        (*ptrNode)->data->type = EXP_VARIABLE;
        (*ptrNode)->data->data.number = 0;
        break;
    case LEX_NUMBER:
        *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
        (*ptrNode)->data = (NodeInfo*)calloc(1, sizeof(NodeInfo));
        (*ptrNode)->data->type = EXP_NUMBER;
        (*ptrNode)->data->data.number = tokens[start].data.value;
        break;
    case LEX_FUNCTION:
        *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
        (*ptrNode)->data = (NodeInfo*)calloc(1, sizeof(NodeInfo));
        (*ptrNode)->data->type = EXP_FUNCTION;
        (*ptrNode)->data->data.opNumber = static_cast<int>(tokens[start].data.value);
        start++;
        p = start;
        do
        {
            if (tokens[p].data.symbol == '(') brackets++;
            if (tokens[p].data.symbol == ')') brackets--;
            p++;
        } while (brackets && p<end);
        p--;
        parse_expr(&(*ptrNode)->link[0], start + 1, p - 1);
        break;
    default:
        Assert_c("Undefined token type.");
        $$$("Undefined token type.");
        return;
    }
    $$
}

static bool isEqualTo(C_string str, C_string sub)
{
    while (*sub && *str)
    {
        if (*str != *sub)
            return 0;
        str++;
        sub++;
    }
    return !*sub;
}

Parser::Token Parser::getNextToken(C_string& str)
{
    $
    static char buff[16] = {};
    Token result;
    while (*str && strchr(" ", str[0]))
        str++;
    if (strchr("()", str[0]))
    {
        result.type = LEX_BRACKET;
        result.data.symbol = str[0];
        str++;
        $$
        return result;
    }
    
    
    for (ui8 i = 0; i < sizeof(function_names) / sizeof(function_names[0]); i++)
    {
        if (isEqualTo(str, function_names[i]))
        {
            result.type = LEX_FUNCTION;
            result.data.value = i;
            str += strlen(function_names[i]);
            $$
            return result;
        }
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
    sscanf(str, "%lf%n", &result.data.value, &offset);
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
        if (it->type == LEX_NUMBER)
            printf("data: %lf\n", it->data.value);
        if (it->type == LEX_OPERATION)
            printf("data: %c\n", it->data.symbol);
        if (it->type == LEX_FUNCTION)
            printf("data: %s\n", function_names[(int)it->data.value]);
    }
    #endif

    parse_expr(&root, 0, tokens.size() - 1);

    $$
    return root;
}

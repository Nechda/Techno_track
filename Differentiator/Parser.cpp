#include "Parser.h"
#include "CallStack.h"

Parser::~Parser()
{$

    tokens.clear();
$$
}

/*
    E -> E + T | E - T | T
    T -> T * T | T / T | T ^ T | F
    F -> N     | (E)   | -(E)  | -T
*/


ui32 Parser::isOperation(ui8 opType, ui32 start, ui32 end)
{$
    ui32 p = end;
    ui32 brackets = 0;
    while (start <= p && p<= end)
    {

        if (tokens[p].ptrToData.symbol == ')')
            do
            {
                if (tokens[p].ptrToData.symbol == ')') brackets++;
                if (tokens[p].ptrToData.symbol == '(') brackets--;
                p--;
            } while (brackets && p<end && p >start);

        if (p > end || p < start) break;

        if (tokens[p].type != LEX_OPERATION)
        {
            p--;
            continue;
        }
        if (tokens[p].ptrToData.symbol == opType)
        {
            $$
            return p;
        }
        p--;
    }
    $$
    return -1;
}

void Parser::parse_expr(Expression::TNode** ptrNode,ui32 start, ui32 end, Expression::TNode* parent)
{$
    ui32 opPos = 0;
    static const ui8 operations[2] = { '+', '-' };

    for (ui8 i = 0; i < 2; i++)
    {
        opPos = isOperation(operations[i], start, end);
        if (opPos != -1)
        {
            *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
            (*ptrNode)->ptrToData = (NodeInfo*)calloc(1, sizeof(NodeInfo));
            (*ptrNode)->ptrToData->type = NODE_TYPE_OPERATION;
            (*ptrNode)->ptrToData->data.opNumber = OP_SUM + i;
            (*ptrNode)->parent = parent;

            parse_expr(&(*ptrNode)->link[0], start, opPos - 1, *ptrNode);
            parse_term(&(*ptrNode)->link[1], opPos + 1, end,   *ptrNode);
            $$
            return;
        }
    }

    parse_term(ptrNode, start, end, parent);
    $$
}

void Parser::parse_term(Expression::TNode** ptrNode, ui32 start, ui32 end, Expression::TNode* parent)
{$
    ui32 opPos = 0;
    static const ui8 operations[3] = { '*', '/', '^' };

    for (ui8 i = 0; i < 3; i++)
    {
        opPos = isOperation(operations[i], start, end);
        if (opPos != -1)
        {
            *ptrNode = (Expression::TNode*)calloc(1, sizeof(Expression::TNode));
            (*ptrNode)->ptrToData = (NodeInfo*)calloc(1, sizeof(NodeInfo));
            (*ptrNode)->ptrToData->type = NODE_TYPE_OPERATION;
            (*ptrNode)->ptrToData->data.opNumber = OP_MUL + i;
            (*ptrNode)->parent = parent;

            parse_term(&(*ptrNode)->link[0], start, opPos - 1, *ptrNode);
            //parse_fact(&(*ptrNode)->link[1], opPos + 1, end);
            parse_term(&(*ptrNode)->link[1], opPos + 1, end, *ptrNode);
            $$
            return;
        }
    }

    parse_fact(ptrNode, start, end, parent);
   $$
}

void Parser::parse_fact(Expression::TNode** ptrNode, ui32 start, ui32 end, Expression::TNode* parent)
{$
    ui32 brackets = 0;
    ui32 p = start;
    switch (tokens[start].type)
    {
    case LEX_BRACKET:
        do
        {
            if (tokens[p].ptrToData.symbol == '(') brackets++;
            if (tokens[p].ptrToData.symbol == ')') brackets--;
            p++;
        } while (brackets && p<end);
        p--;
        parse_expr(ptrNode, start + 1, p - 1, parent);
        break;
    case LEX_VARIABLE:
        createNode(*ptrNode, NODE_TYPE_VARIABLE);
        (*ptrNode)->ptrToData->data.number = 0;
        (*ptrNode)->parent = parent;
        break;
    case LEX_NUMBER:
        createNode(*ptrNode, NODE_TYPE_NUMBER);
        (*ptrNode)->ptrToData->data.number = tokens[start].ptrToData.value;
        (*ptrNode)->parent = parent;
        break;
    case LEX_FUNCTION:
        createNode(*ptrNode, NODE_TYPE_FUNCTION);
        (*ptrNode)->ptrToData->data.opNumber = static_cast<int>(tokens[start].ptrToData.value);
        (*ptrNode)->parent = parent;
        start++;
        p = start;
        do
        {
            if (tokens[p].ptrToData.symbol == '(') brackets++;
            if (tokens[p].ptrToData.symbol == ')') brackets--;
            p++;
        } while (brackets && p<end);
        p--;
        if (start != p)
        {
            parse_expr(&(*ptrNode)->link[0], start + 1, p, *ptrNode);
            break;
        }

        //мы наткнулись на функцию, без скобок, т.е унарный минус
        brackets = 0;
        do
        {
            if (tokens[p].ptrToData.symbol == '(') brackets++;
            if (tokens[p].ptrToData.symbol == ')') brackets--;
            p++;
            if (p > end) break;
        } while ((tokens[p].type != LEX_OPERATION || brackets) && p <= end);
        parse_fact(&(*ptrNode)->link[0], start, p, *ptrNode);

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
        result.ptrToData.symbol = str[0];
        str++;
        $$
        return result;
    }
    
    
    for (ui8 funcIndex = 0; funcIndex < FUNCTION_TABLE_SIZE; funcIndex++)
    {
        if (isEqualTo(str, FUNCTION_NAMES_TABLE[funcIndex]))
        {
            result.type = LEX_FUNCTION;
            result.ptrToData.value = funcIndex;
            str += strlen(FUNCTION_NAMES_TABLE[funcIndex]);
            $$
            return result;
        }
    }
    

    if (str[0] == 'x')
    {
        result.type = LEX_VARIABLE;
        result.ptrToData.symbol = str[0];
        str++;
        $$
        return result;
    }


    if (strchr("+-*/^", str[0]))
    {
        result.type = LEX_OPERATION;
        result.ptrToData.symbol = str[0];
        str++;
        $$
        return result;
    }

    ui32 offset = 0;
    sscanf(str, "%lf%n", &result.ptrToData.value, &offset);
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

    bool wasSpaceOrBracket = 1;
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (it->type == LEX_OPERATION && it->ptrToData.symbol == '-' && wasSpaceOrBracket)
        {
            it->type = LEX_FUNCTION;
            it->ptrToData.value = FUNC_NEG;
        }
        wasSpaceOrBracket = it->ptrToData.symbol == '(';
    }


    #ifndef NDEBUG
    ui32 counter = 0;
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (it->type == LEX_NUMBER)
            printf("[%d] data: %lf\n",counter, it->ptrToData.value);
        if (it->type == LEX_OPERATION || it->type == LEX_BRACKET)
            printf("[%d] data: %c\n", counter, it->ptrToData.symbol);
        if (it->type == LEX_VARIABLE)
            printf("[%d] data: x\n", counter);
        if (it->type == LEX_FUNCTION)
            printf("[%d] data: %s\n", counter, FUNCTION_NAMES_TABLE[(int)it->ptrToData.value]);
        counter++;
    }
    #endif

    parse_expr(&root, 0, tokens.size() - 1, NULL);

    $$
    return root;
}

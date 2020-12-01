#include "Parser.h"
#include "CallStack.h"

Parser::~Parser()
{$

    tokens.clear();
$$
}

/*
    E -> E + E | E - E | T
    T -> T * T | T / T | T ^ T | F
    F -> N     | (E)   | -(E)  | -F
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
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, OP_SUM + i, parent);
            parse_expr(&(*ptrNode)->link[0], start, opPos - 1, *ptrNode);
            parse_expr(&(*ptrNode)->link[1], opPos + 1, end,   *ptrNode);
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
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, OP_MUL + i, parent);
            parse_term(&(*ptrNode)->link[0], start, opPos - 1, *ptrNode);
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
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE, 0, parent);
        break;
    case LEX_NUMBER:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_NUMBER, tokens[start].ptrToData.value, parent);
        break;
    case LEX_FUNCTION:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_FUNCTION, tokens[start].ptrToData.value, parent);
        start++;
        p = start;
        do
        {
            if (tokens[p].ptrToData.symbol == '(') brackets++;
            if (tokens[p].ptrToData.symbol == ')') brackets--;
            p++;
        } while (brackets && p<=end);
        p--;
        if (start != p)
        {
            parse_expr(&(*ptrNode)->link[0], start + 1, p-1, *ptrNode);
            break;
        }

        //мы наткнулись на функцию, без скобок, т.е унарный минус
        brackets = 0;
        do
        {
            if (tokens[p].ptrToData.symbol == '(') brackets++;
            if (tokens[p].ptrToData.symbol == ')') brackets--;
            p++;
            //if (p > end) break;
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
    if (!offset)
        result.type = LEX_UNDEFINED;
    else
        result.type = LEX_NUMBER;
    str += offset;
    $$
    return result;
}

Expression::TNode* Parser::parse(C_string expression)
{
    $
    Expression::TNode* root = NULL;
    tokens.clear();

    while (*expression)
    {
        tokens.push_back(getNextToken(expression));
        if (tokens.back().type == LEX_UNDEFINED)
        {
            Assert_c(!"Error occur: find undefinded token.");
            return NULL;
        }
    }

    //stage 1: checking bracket sequence
    {
        ui16 brackets = 0;
        bool isInvalidBracketSequence = 0;
        for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end() && !isInvalidBracketSequence; it++)
        {
            if (it->type == LEX_BRACKET && it->ptrToData.symbol == '(') brackets++;
            if (it->type == LEX_BRACKET && it->ptrToData.symbol == ')') brackets--;
            if (brackets < 0)
                isInvalidBracketSequence |= 1;
        }
        isInvalidBracketSequence |= brackets;
        if (isInvalidBracketSequence)
        {
            Assert_c(!"Error occur: incorrect placement of brackets");
            return NULL;
        }
    }

    //stage 2: сhecking lexema sequence
    {
        bool canNext = 0;
        bool canPrev = 0;
        bool isInvalidTokenSequence = 0;
        ui32 nTokens = tokens.size();
        for (ui32 i = 0; i < nTokens; i++)
        {
            isInvalidTokenSequence = 0;
            canNext = i < nTokens - 1;
            canPrev = i > 0;
            switch (tokens[i].type)
            {
                case LEX_VARIABLE:
                case LEX_NUMBER:
                    if (canNext)
                        isInvalidTokenSequence |= !strchr("-+*/^)", tokens[i + 1].ptrToData.symbol);
                    if (canPrev)
                        isInvalidTokenSequence |= !strchr("-+*/^(", tokens[i - 1].ptrToData.symbol);
                    break;
                case LEX_BRACKET:
                    if (tokens[i].ptrToData.symbol == '(')
                    {
                        if (canNext)
                            isInvalidTokenSequence |= !(
                                tokens[i + 1].ptrToData.symbol == '-' 
                              || tokens[i + 1].type == LEX_NUMBER 
                              || tokens[i + 1].type == LEX_VARIABLE 
                              || tokens[i + 1].type == LEX_FUNCTION
                            );
                        if(canPrev)
                            isInvalidTokenSequence |= !strchr("-+*/^(", tokens[i - 1].ptrToData.symbol);
                    }
                    else
                    {
                        if (canNext)
                            isInvalidTokenSequence |= !strchr("-+*/^)", tokens[i + 1].ptrToData.symbol);
                        if (canPrev)
                            isInvalidTokenSequence |= !(
                                   tokens[i - 1].ptrToData.symbol == ')'
                                || tokens[i - 1].type == LEX_NUMBER
                                || tokens[i - 1].type == LEX_VARIABLE
                                );
                    }
                    break;
                case LEX_FUNCTION:
                    if(canNext)
                        isInvalidTokenSequence |= tokens[i + 1].ptrToData.symbol != '(';
                    if(canPrev)
                        isInvalidTokenSequence |= !strchr("-+*/^(", tokens[i - 1].ptrToData.symbol);
                    break;
                case LEX_OPERATION:
                    if (canNext)
                        isInvalidTokenSequence |= !(
                               tokens[i + 1].ptrToData.symbol == '('
                            || tokens[i + 1].type == LEX_NUMBER
                            || tokens[i + 1].type == LEX_VARIABLE
                            || tokens[i + 1].type == LEX_FUNCTION
                        );
                    if(canPrev)
                        isInvalidTokenSequence |= !(
                            tokens[i - 1].ptrToData.symbol == ')'
                            || tokens[i - 1].type == LEX_NUMBER
                            || tokens[i - 1].type == LEX_VARIABLE
                            );
                    break;
                default:
                    isInvalidTokenSequence |= 1;
                    break;
            }
            if(isInvalidTokenSequence)
            {
                Assert_c(!"Error occur: incorrect placement of lexems");
                return NULL;
            }
        }
    }

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


    #ifdef PARSER_DEBUG
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

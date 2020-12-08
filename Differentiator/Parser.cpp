#include "Parser.h"
#include "CallStack.h"

Parser::~Parser()
{$

    tokens.clear();
$$
}

/*
    E -> T + E | T - E | T
    T -> F * T | F / F | F ^ T | F
    F -> N     | (E)   | -(E)  | -F
*/

void Parser::parse_expr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
    static const i8* operations = "+-";
    Expression::TNode* link = NULL;

    parse_term(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$
        return;
    }

    ui32 op = (ui32)strchr(operations, tokens[p].ptrToData.symbol);
    op = op ? op - (ui32)operations : -1;

    if (op!=-1)
    {
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, OP_SUM + op, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        parse_expr(&(*ptrNode)->link[1], p, *ptrNode);
        
    }
    else
        (*ptrNode) = link;

    $$
    return;
}

void Parser::parse_term(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
    static const i8* operations = "*/^";
    Expression::TNode* link = NULL;

    parse_fact(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$
        return;
    }

    ui32 op = (ui32)strchr(operations, tokens[p].ptrToData.symbol);
    op = op ? op - (ui32)operations : -1;

    if (op != -1)
    {
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, OP_MUL + op, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        if(OP_MUL + op == OP_DIV)
            parse_fact(&(*ptrNode)->link[1], p, *ptrNode);
        else
            parse_term(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
    return;
}

void Parser::parse_fact(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
    switch (tokens[p].type)
    {
        case LEX_BRACKET:
            p++;
            parse_expr(ptrNode, p, parent);
            p++;
            break;
        case LEX_VARIABLE:
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE, 0, parent);
            p++;
            break;
        case LEX_NUMBER:
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_NUMBER, tokens[p].ptrToData.value, parent);
            p++;
            break;
        case LEX_FUNCTION:
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_FUNCTION, tokens[p].ptrToData.value, parent);
            if (tokens[p + 1].type == LEX_BRACKET)
            {
                p+=2;
                parse_expr(&(*ptrNode)->link[0], p, *ptrNode);
                p++;
                break;
            }
            parse_fact(&(*ptrNode)->link[0], p, *ptrNode);
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
                printf("Error occur: incorrect placement of lexems\n");
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
    ui32  p = 0;
    parse_expr(&root, p, NULL);

    $$
    return root;
}

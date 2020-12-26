#include "Parser.h"
#include "CallStack.h"
#include "Hash.h"


//табличка со всеми токенами языка
const triple<C_string, Parser::LexemaType, OpType> Parser::tokensTable[] =
{
    { "(",    TOKEN_BRACKET,                OP_UNDEFINED },
    { ")",    TOKEN_BRACKET,                OP_UNDEFINED },
    { "{",    TOKEN_CURLY_BACKET,           OP_UNDEFINED },
    { "}",    TOKEN_CURLY_BACKET,           OP_UNDEFINED },
    { ";",    TOKEN_SEMICOLON,              OP_SEMICOLON },
    { "+",    TOKEN_ARITHMETIC_OPERATION,   OP_SUM },
    { "-",    TOKEN_ARITHMETIC_OPERATION,   OP_SUB },
    { "*",    TOKEN_ARITHMETIC_OPERATION,   OP_MUL },
    { "/",    TOKEN_ARITHMETIC_OPERATION,   OP_DIV },
    { "^",    TOKEN_ARITHMETIC_OPERATION,   OP_POW },
    { "var",  TOKEN_VARIABLE_TYPE,          OP_UNDEFINED },
    { "if",   TOKEN_IF,                     OP_BRANCH },
    { "else", TOKEN_ELSE,                   OP_UNDEFINED },
    { "||",   TOKEN_OR,                     OP_OR },
    { "&&",   TOKEN_AND,                    OP_AND },
    { ">=",   TOKEN_LOGIC_OPERATION,        OP_GAIN_OR_EQUAL },
    { "<=",   TOKEN_LOGIC_OPERATION,        OP_LESS_OR_EQUAL },
    { "==",   TOKEN_LOGIC_OPERATION,        OP_EQUAL },
    { "!=",   TOKEN_LOGIC_OPERATION,        OP_NEQUAL },
    { "=",    TOKEN_ASSIGMENT,              OP_ASSIGMENT },
    { ">",    TOKEN_LOGIC_OPERATION,        OP_GAIN },
    { "<",    TOKEN_LOGIC_OPERATION,        OP_LESS },
    { "def",  TOKEN_DEF,                    OP_DEF },
    { ",",    TOKEN_COMMA,                  OP_COMMA },
    { "$",    TOKEN_UNDEFINED,              OP_DOLLAR }, /// <--- этот токен только для объеднинения функций
    { "ret",  TOKEN_RETURN,                 OP_RETURN },
    { "while",TOKEN_WHILE,                  OP_UNDEFINED}

};
const ui8 Parser::TOKENS_TABLE_SIZE = sizeof(Parser::tokensTable) / sizeof(Parser::tokensTable[0]);

ui8 Parser::getTokenIndexByString(C_string str)
{
    for (ui8 i = 0; i < TOKENS_TABLE_SIZE; i++)
        if (!strcmp(str, tokensTable[i].first))
            return i;
    return -1;
}


Parser::~Parser()
{
    $

        tokens.clear();
    $$
}


/*
    Fl - File
    Fu - Function
    fN - Function name
    Ar - Arguments of function
    Av - Portable arguments
    G  - General (base block of code)
    L  - Line
    B  - Branch
    V  - Variable
    vT - Variable type
    vN - Variable name
    LE - Logical expression (number means order)
    E  - Expression
    T  - Term (?)
    D  - Divider
    F  - Factor
    N  - Can be function, number or variable
    Fl -> Fu Fl | Fu
    Fu -> def fN(Ar){G}
    Ar -> vT vN , Ar | vT vN |
    G  -> L ; G | B G
    B  -> if(LE1){G}else{G} | if(LE1){G}
    L  -> V = LE1 | LE1
    V  -> vT vN | vN
    LE1 -> LE2 || LE1 | LE2
    LE2 -> LE3 && LE2 | LE3
    LE3 -> E < E | E > E | E <= E | E >= E | E == E | E != E | E = E
    E -> T + E | T - E | T
    T -> D * T | D ^ T | D
    D -> F / F | F
    F -> N     | (LE1)   | -(LE1)  | -F
*/

/*
    tokens[] содержит 2 поля:
    type --- тип токена, т.е. enum, описанный в таблице выше.
    может быть, например, TOKEN_OR, TOKEN_DEF, ...
    dataUnion --- объединение double и ui32, при записи числа в узел дерева
    считывается поле dvalue, в противном случае необходимо работать с ivalue
    причем ivalue хранит в себе ИНДЕКС в таблице токенов, а не enum операции!!!
*/

static i32 findOperation(const ui8* arr, ui8 value)
{
    i32 index = 0;
    while (arr[index] != 255)
    {
        if (arr[index] == value)
            return index;
        index++;
    }
    return -1;
}


void Parser::parse_file(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        Expression::TNode* link = NULL;
    parse_function(&link, p, parent);

    if (!link)
    {
        $$$("Invalid function definition");
        return;
    }

    if (p >= tokens.size())
    {
        *ptrNode = link;
        $$
            return;
    }

    UnionData unionData;

    unionData.ivalue = OP_DOLLAR;
    *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, unionData, parent);
    parse_file(&(*ptrNode)->link[1], p, *ptrNode);

    $$
        return;
}

void Parser::parse_function(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
    ui32 maxSize = tokens.size();
    #define checkErrors()\
     if(isErrorOccur)\
     {\
        $$$("Invalid sequence of lexemas.");\
        if (*ptrNode) rCleanUp(*ptrNode);\
        *ptrNode = NULL;\
        return;\
     } 
    #define safeINC(x) (x)++; if((x) >= maxSize) {$$$("Out of range");return;}
    UnionData unionData;

    unionData.ivalue = OP_DEF;
    bool isErrorOccur = 0;
    isErrorOccur |= !(tokens[p].type == TOKEN_DEF);
    checkErrors();
    safeINC(p);

    *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);

    isErrorOccur |= !(tokens[p].type == TOKEN_NAME);
    unionData = tokens[p].dataUnion;
    checkErrors();
    safeINC(p);
    (*ptrNode)->link[0] = createNode(NULL, NULL, NODE_TYPE_NAME, unionData, *ptrNode);

    isErrorOccur |= !(tokens[p].type == TOKEN_BRACKET);
    checkErrors();
    safeINC(p);
    parse_arguments(&(*ptrNode)->link[1], p, *ptrNode);
    isErrorOccur |= !(tokens[p].type == TOKEN_BRACKET);
    checkErrors();
    safeINC(p);

    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);
    parse_general(&(*ptrNode)->link[2], p, *ptrNode);
    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);
}

void Parser::parse_arguments(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        Expression::TNode* link = NULL;
    UnionData unionData;

    if (tokens[p].type == TOKEN_VARIABLE_TYPE)
    {
        link = createNode(NULL, NULL, NODE_TYPE_VARIABLE_SPECIFICALOR, tokens[p].dataUnion, parent);
        p++;
        parse_fact(&link->link[0], p, link);
    }
    else
    {
        $$
            return;
    }
    if (p >= tokens.size())
    {
        $$$("Out of range tokens array");
        return;
    }

    if (tokens[p].type == TOKEN_COMMA)
    {
        unionData.ivalue = OP_COMMA;
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, unionData, parent);
        p++;
        parse_arguments(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        *ptrNode = link;
    $$
        return;
}

void Parser::parse_argumentsValue(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        Expression::TNode* link = NULL;
    UnionData unionData;

    parse_logicExpr1(&link, p, link);

    if (p >= tokens.size())
    {
        $$$("Out of range tokens array");
        return;
    }

    if (tokens[p].type == TOKEN_COMMA)
    {
        unionData.ivalue = OP_COMMA;
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, unionData, parent);
        p++;
        parse_argumentsValue(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        *ptrNode = link;
    $$
        return;
}

void Parser::parse_general(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        static const ui8 operations[] = { getTokenIndexByString(";"), -1 };
    Expression::TNode* link = NULL;

    if (p >= tokens.size()) { $$ return; }
    parse_line(&link, p, parent);

    if (tokens[p].type != TOKEN_SEMICOLON && link)
    {
        $$$("Skipped ;");
        return;
    }
    while (link && p < tokens.size() && tokens[p].type == TOKEN_SEMICOLON)
        p++;


    UnionData unionData;
    unionData.ivalue = OP_SEMICOLON;

    bool throwLine = 0;
    throwLine |= p >= tokens.size();
    throwLine |= throwLine ? 1 : tokens[p].type == TOKEN_CURLY_BACKET;

    if (throwLine)
    {
        (*ptrNode) = link;
        $$
            return;
    }

    if (!link)
    {
        if (tokens[p].type == TOKEN_IF)
            parse_branch(&link, p, parent);
        else if (tokens[p].type == TOKEN_WHILE)
            parse_loop(&link, p, parent);
        else
        {
            $$$("Undefined lexema.");
            return;
        }
    }

    Expression::TNode* tmpLink = NULL;
    parse_general(&tmpLink, p, parent);

    if (tmpLink)
        *ptrNode = createNode(link, tmpLink, NODE_TYPE_OPERATION, unionData, parent);
    else
        *ptrNode = link;

    $$
        return;
}

void Parser::parse_line(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        static const ui8 operations[] = { getTokenIndexByString("="), -1 };
    Expression::TNode* link = NULL;

    parse_var(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$
            return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1 && link)
    {
        UnionData unionData;
        unionData.ivalue = OP_ASSIGMENT;
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        parse_logicExpr1(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
    {
        parse_logicExpr1(&link, p, parent);
        (*ptrNode) = link;
    }

    $$
        return;
}

void Parser::parse_branch(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        ui32 maxSize = tokens.size();
#define checkErrors()\
     if(isErrorOccur)\
     {\
        $$$("Invalid sequence of lexemas.");\
        if (*ptrNode) rCleanUp(*ptrNode);\
        *ptrNode = NULL;\
        return;\
     } 
#define safeINC(x) (x)++; if((x) >= maxSize) {$$$("Out of range");return;}
    UnionData unionData;


    unionData.ivalue = OP_BRANCH;
    bool isErrorOccur = 0;
    isErrorOccur |= !(tokens[p].type == TOKEN_IF);
    checkErrors();
    safeINC(p);
    isErrorOccur |= !(tokens[p].type == TOKEN_BRACKET);
    checkErrors();
    safeINC(p);

    *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);

    parse_logicExpr1(&(*ptrNode)->link[0], p, *ptrNode);
    isErrorOccur |= !(*ptrNode)->link[0];
    checkErrors();
    isErrorOccur |= !(tokens[p].type == TOKEN_BRACKET);
    checkErrors();
    safeINC(p);


    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);
    parse_general(&(*ptrNode)->link[1], p, *ptrNode);
    isErrorOccur |= !(*ptrNode)->link[1];
    checkErrors();
    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);

    if (tokens[p].type != TOKEN_ELSE)
    {
        $$
            return;
    }
    safeINC(p);

    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);
    parse_general(&(*ptrNode)->link[2], p, *ptrNode);
    isErrorOccur |= !(*ptrNode)->link[2];
    checkErrors();
    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);
}

void Parser::parse_loop(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    ui32 maxSize = tokens.size();
    #define checkErrors()\
     if(isErrorOccur)\
     {\
        $$$("Invalid sequence of lexemas.");\
        if (*ptrNode) rCleanUp(*ptrNode);\
        *ptrNode = NULL;\
        return;\
     } 
    #define safeINC(x) (x)++; if((x) >= maxSize) {$$$("Out of range");return;}
    UnionData unionData;


    unionData.ivalue = OP_WHILE;
    bool isErrorOccur = 0;
    isErrorOccur |= !(tokens[p].type == TOKEN_WHILE);
    checkErrors();
    safeINC(p);
    isErrorOccur |= !(tokens[p].type == TOKEN_BRACKET);
    checkErrors();
    safeINC(p);

    *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);

    parse_logicExpr1(&(*ptrNode)->link[0], p, *ptrNode);
    isErrorOccur |= !(*ptrNode)->link[0];
    checkErrors();
    isErrorOccur |= !(tokens[p].type == TOKEN_BRACKET);
    checkErrors();
    safeINC(p);


    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);
    parse_general(&(*ptrNode)->link[1], p, *ptrNode);
    isErrorOccur |= !(*ptrNode)->link[1];
    checkErrors();
    isErrorOccur |= !(tokens[p].type == TOKEN_CURLY_BACKET);
    checkErrors();
    safeINC(p);
}

void Parser::parse_var(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        switch (tokens[p].type)
        {
        case TOKEN_VARIABLE_TYPE:
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE_SPECIFICALOR, tokens[p].dataUnion, parent);
            p++;
            parse_fact(&(*ptrNode)->link[0], p, *ptrNode);
            break;
        case TOKEN_NAME:
            if (tokens[p + 1].type != TOKEN_BRACKET)
            {
                *ptrNode = createNode(NULL, NULL, NODE_TYPE_NAME, tokens[p].dataUnion, parent);
                p++;
            }
            break;
        default:
            Assert_c("Undefined token type.");
            $$$("Undefined token type.");
            return;
        }
    $$
}

void Parser::parse_logicExpr1(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        static const ui8 operations[] = { getTokenIndexByString("||"), -1 };
    Expression::TNode* link = NULL;

    parse_logicExpr2(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$
            return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        UnionData unionData;
        unionData.ivalue = OP_OR;
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        parse_logicExpr1(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
        return;
}

void Parser::parse_logicExpr2(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        static const ui8 operations[] = { getTokenIndexByString("&&"), -1 };
    Expression::TNode* link = NULL;

    parse_logicExpr3(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$
            return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        UnionData unionData;
        unionData.ivalue = OP_AND;
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        parse_logicExpr2(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
        return;
}

void Parser::parse_logicExpr3(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        static const ui8 operations[] = {
        getTokenIndexByString(">"),getTokenIndexByString("<"),
        getTokenIndexByString(">="),getTokenIndexByString("<="),
        getTokenIndexByString("=="),getTokenIndexByString("!="),
        getTokenIndexByString("="),-1 };
    Expression::TNode* link = NULL;

    parse_expr(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$ return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        UnionData unionData;
        unionData.ivalue = tokensTable[operations[op]].third;
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        parse_expr(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
        return;
}


void Parser::parse_expr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        static const ui8 operations[] = { getTokenIndexByString("+"), getTokenIndexByString("-"), -1 };
    Expression::TNode* link = NULL;

    parse_term(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$
            return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        UnionData unionData;
        unionData.ivalue = tokensTable[operations[op]].third;
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
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
        static const ui8 operations[] = { getTokenIndexByString("*"), getTokenIndexByString("^"), -1 };
    Expression::TNode* link = NULL;

    parse_divider(&link, p, parent);
    if (tokens.size() <= p || !link)
    {
        (*ptrNode) = link;
        $$ return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        UnionData unionData;
        unionData.ivalue = tokensTable[operations[op]].third;
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        parse_term(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$ return;
}

void Parser::parse_divider(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        static const ui8 operations[] = { getTokenIndexByString("/"), -1 };
    Expression::TNode* link = NULL;

    parse_fact(&link, p, parent);
    if (tokens.size() <= p)
    {
        (*ptrNode) = link;
        $$
            return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        UnionData unionData;
        unionData.ivalue = OP_DIV;
        p++;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
        (*ptrNode)->link[0] = link;
        link->parent = (*ptrNode);
        parse_fact(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
        return;
}

void Parser::parse_fact(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{
    $
        Expression::TNode* link = NULL;
    UnionData dataUnion;
    switch (tokens[p].type)
    {
    case TOKEN_BRACKET:
        p++;
        parse_logicExpr1(ptrNode, p, parent);
        p++;
        break;
    case TOKEN_VARIABLE_TYPE:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE_SPECIFICALOR, tokens[p].dataUnion, parent);
        p++;
        break;
    case TOKEN_NAME:
        link = createNode(NULL, NULL, NODE_TYPE_NAME, tokens[p].dataUnion, parent);
        p++;
        if (tokens[p].type == TOKEN_BRACKET && tokens[p].dataUnion.ivalue == 0) /// открывается скобка
        {
            p++;
            dataUnion.ivalue = 0;
            *ptrNode = createNode(link, NULL, NODE_TYPE_CUSTOM_FUNCTION, dataUnion, parent);
            parse_argumentsValue(&(*ptrNode)->link[1], p, *ptrNode);
            p++;
        }
        else
            *ptrNode = link;
        break;
    case TOKEN_NUMBER:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_NUMBER, tokens[p].dataUnion, parent);
        p++;
        break;
    case TOKEN_RETURN:
        dataUnion.ivalue = OP_RETURN;
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        p++;
        if (tokens[p].type == TOKEN_SEMICOLON)
        {
            dataUnion.dvalue = 0.0;
            (*ptrNode)->link[0] = createNode(NULL, NULL, NODE_TYPE_NUMBER, dataUnion, *ptrNode);
        }
        else
            parse_logicExpr1(&(*ptrNode)->link[0], p, *ptrNode);
        break;
    case TOKEN_FUNCTION:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_FUNCTION, tokens[p].dataUnion, parent);
        if (tokens[p + 1].type == TOKEN_BRACKET)
        {
            p += 2;
            if (tokens[p].type == TOKEN_BRACKET) {
                p++;
                break;
            }

            parse_logicExpr1(&(*ptrNode)->link[0], p, *ptrNode);
            p++;
            break;
        }
        p++;
        parse_fact(&(*ptrNode)->link[0], p, *ptrNode);
        break;
    default:
        Assert_c("Undefined token type.");
        $$$("Undefined token type.");
        return;
    }
    $$
}

static bool isStrStartFromSubStr(C_string str, C_string sub)
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
        static char buff[32] = {};
    Token result;
    while (*str && strchr(" ", str[0]))
        str++;


    for (ui8 tokenIndex = 0; tokenIndex < TOKENS_TABLE_SIZE; tokenIndex++)
    {
        if (isStrStartFromSubStr(str, tokensTable[tokenIndex].first))
        {
            result.type = tokensTable[tokenIndex].second;
            result.dataUnion.ivalue = tokenIndex;
            str += strlen(tokensTable[tokenIndex].first);
            $$
                return result;
        }
    }


    for (ui8 funcIndex = 0; funcIndex < FUNCTION_TABLE_SIZE; funcIndex++)
    {
        if (isStrStartFromSubStr(str, FUNCTION_NAMES_TABLE[funcIndex]))
        {
            result.type = TOKEN_FUNCTION;
            result.dataUnion.ivalue = funcIndex;
            str += strlen(FUNCTION_NAMES_TABLE[funcIndex]);
            $$
                return result;
        }
    }


    sscanf(str, "%[^ ;=+--*\\/()^{},!<>]%*c", buff);
    bool isNumber = 1;
    ui8 index = 0;
    while (buff[index] && index < 32 && isNumber)
    {
        isNumber &= ('0' <= buff[index] && buff[index] <= '9' || buff[index] == '.');
        index++;
    }

    if (isNumber)
    {
        ui32 offset = 0;
        sscanf(str, "%lf%n", &result.dataUnion.dvalue, &offset);
        if (!offset)
            result.type = TOKEN_UNDEFINED;
        else
            result.type = TOKEN_NUMBER;
        str += offset;
    }
    else
    {
        //если ничего не найдено, то текущий токен задает переменную
        result.type = TOKEN_NAME;
        result.dataUnion.ivalue = getHash(buff, strlen(buff));
        str += strlen(buff);
    }

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
        if (tokens.back().type == TOKEN_UNDEFINED)
        {
            Assert_c(!"Error occur: find undefinded token.");
            return NULL;
        }
    }




    bool wasSpaceOrBracket = 1;
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (it->type == TOKEN_ARITHMETIC_OPERATION && it->dataUnion.ivalue == 6 && wasSpaceOrBracket)
        {
            it->type = TOKEN_FUNCTION;
            it->dataUnion.ivalue = FUNC_NEG;
        }
        wasSpaceOrBracket = !(it->type == TOKEN_NAME || it->type == TOKEN_NUMBER);
    }

    //#define PARSER_DEBUG

#ifdef PARSER_DEBUG
    ui32 counter = 0;
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        printf("[%d] data:", counter);
        switch (it->type)
        {
        case TOKEN_NUMBER:
            printf("%lf", it->dataUnion.dvalue);
            break;
        case TOKEN_NAME:
            printf("VARIABLE_HASH = 0x%X", it->dataUnion.ivalue);
            break;
        case TOKEN_FUNCTION:
            printf("%s", FUNCTION_NAMES_TABLE[it->dataUnion.ivalue]);
            break;
        default:
            printf("%s", tokensTable[it->dataUnion.ivalue].first);
            break;
        }
        printf("\n");
        counter++;

    }
#endif
    ui32  p = 0;
    parse_file(&root, p, NULL);
    $$
        return root;
}

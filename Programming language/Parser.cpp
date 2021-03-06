#include "Parser.h"
#include "Logger.h"
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
{$
    tokens.clear();
    $$ return;
}


/*
    File  - File
    Func  - Function
    Args  - Arguments of function
    Avar  - Enumeration of the elements
            (uses as sequence of expression, divided by ",",
            that interpretates as function's arguments)
    Blck  - Block of code
    Line  - Line of the code
    Brch  - Branch implements if-else construction
    Loop  - Loop implements while loop
    Var   - Variable definition
    OrLE  - Logic expression with "or" operator
    AndLE - Logic expression with "and" operator
    CmpLe - Logic expression that is consisting compare operators
    AddE  - Additive exprression
    MulE  - Multiplicative exprression
    FraE  - Logic expression with "div" operator
    UnarE - Expression that is consisting unary operations
    Oprnd - Operand
    Nopr  - Named operand (variable or function name)
    Name  - A sequence of characters
    Numbr - Number
    STDF  - Standart language function

    File  -> Func File | Func
    Func  -> def Name(Args){G}
    Args  -> Var  , Args | Var        |
    Blck  -> Line ; Blck | Brch  Blck | Loop Blck
    Line  -> ret    OrLE | Var = OrLE | OrLE
    Brch  -> if(OrLE){Blck}else{Blck} | if(OrLE){Blck}
    Loop  -> while(OrLE){Blck}
    Var   -> vT Name

    OrLE  -> AndLE || AndLE | AndLE
    AndLE -> CmpLe && CmpLe | CmpLe
    CmpLe -> AddE  <  AddE  | AddE  >  AddE
             AddE  <= AddE  | AddE  >= AddE
             AddE  == AddE  | AddE  != AddE
             AddE  =  AddE

    AddE  -> MulE  +  AddE  | MulE  -  AddE  | MulE
    MulE  -> DivE  *  MulE  | DivE  ^  MulE  | DivE
    FraE  -> UnarE /  UnarE | UnarE
    UnarE ->       -  Oprnd | Oprnd
    Oprnd -> Numbr |  Nopr  | (OrLE)

    Nopr  -> Name  | Name(Avar) | STDF(OrLE)
*/

#define Assert_parser( expr_, msg )\
    if(!(expr_))\
    {\
        logger("Parser error","%s", msg);\
        $$$("Invalid sequence of lexemas: %s", msg);\
        if (*ptrNode) rCleanUp(*ptrNode);\
        *ptrNode = NULL;\
        return;\
    }

#define standartVariableBlock\
    ui32 maxSize = tokens.size();\
    UnionData dataUnion;\

#define $$return\
    {$$ return; }

#define safeINC(x)\
    {\
        (x)++;\
        if((x) >= maxSize)\
        {\
            $$$("Out of range");\
            return;\
        }\
    }


/*
    tokens[] содержит 2 поля:
    type --- тип токена, т.е. enum, описанный в таблице выше.
    может быть, например, TOKEN_OR, TOKEN_DEF, ...
    dataUnion --- объединение double и ui32, при записи числа в узел дерева
    считывается поле dvalue, в противном случае необходимо работать с ivalue
    причем ivalue хранит в себе ИНДЕКС в таблице токенов, а не enum операции!!!

    в частности, если type == TOKEN_FUNCTION, то поле dataUnion.ivalue содержит
    enum данной функции
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
{$
    standartVariableBlock;
    Expression::TNode* link = NULL;
    parse_function(&link, p, parent);

    if (!link)
    {
        $$$("Invalid function definition");
        return;
    }

    if (maxSize <= p)
    {
        *ptrNode = link;
        $$return;
    }

    dataUnion.ivalue = OP_DOLLAR;
    *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
    parse_file(&(*ptrNode)->link[1], p, *ptrNode);

    $$return;
}

void Parser::parse_function(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;

    dataUnion.ivalue = OP_DEF;
    Assert_parser(tokens[p].type == TOKEN_DEF, "The \'def\' keyword was expected.");
    safeINC(p);

    *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, dataUnion, parent);

    Assert_parser(tokens[p].type == TOKEN_NAME, "The function name was expected.");
    dataUnion = tokens[p].dataUnion;
    safeINC(p);
    (*ptrNode)->link[0] = createNode(NULL, NULL, NODE_TYPE_NAME, dataUnion, *ptrNode);

    Assert_parser(tokens[p].type == TOKEN_BRACKET, "\'(\' was expected.");
    safeINC(p);
    parse_arguments(&(*ptrNode)->link[1], p, *ptrNode);
    Assert_parser(tokens[p].type == TOKEN_BRACKET, "\')\' was expected.");
    safeINC(p);

    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'{\' was expected.");
    safeINC(p);
    parse_codeblock(&(*ptrNode)->link[2], p, *ptrNode);
    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'}\' was expected.");
    safeINC(p);
}

void Parser::parse_arguments(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    Expression::TNode* link = NULL;

    Assert_parser(tokens[p].type == TOKEN_VARIABLE_TYPE, "The \'var\' keyword was expected.");
    parse_var(&link, p, parent);

    if (tokens[p].type == TOKEN_COMMA)
    {
        dataUnion.ivalue = OP_COMMA;
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        safeINC(p);
        parse_arguments(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        *ptrNode = link;

    $$return;
}

void Parser::parse_argumentsValue(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    Expression::TNode* link = NULL;

    parse_ORLogicExpr(&link, p, link);
    Assert_c(p < maxSize, "The file ended unexpectedly.");
    if (tokens[p].type == TOKEN_COMMA)
    {
        dataUnion.ivalue = OP_COMMA;
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        safeINC(p);
        parse_argumentsValue(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        *ptrNode = link;
    $$return;
}

void Parser::parse_codeblock(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = { getTokenIndexByString(";"), -1 };
    Expression::TNode* link = NULL;

    if (maxSize <= p)
        $$return;

    if (!link)
        parse_branch(&link, p, parent);
    if (!link)
        parse_loop(&link, p, parent);
    if (!link)
    {
        parse_line(&link, p, parent);
        Assert_parser(link, "Parser couldn\'t parse the current line of code.");
        Assert_parser(tokens[p].type == TOKEN_SEMICOLON, "\';\' was expected.");
    }
    while (p < tokens.size() && tokens[p].type == TOKEN_SEMICOLON)
        safeINC(p);


    dataUnion.ivalue = OP_SEMICOLON;

    bool throwLine = 0;
    throwLine |= p >= tokens.size();
    throwLine |= throwLine ? 1 : tokens[p].type == TOKEN_CURLY_BACKET;

    if (throwLine)
    {
        (*ptrNode) = link;
        $$return;
    }

    Expression::TNode* tmpLink = NULL;
    parse_codeblock(&tmpLink, p, parent);

    if (tmpLink)
        *ptrNode = createNode(link, tmpLink, NODE_TYPE_OPERATION, dataUnion, parent);
    else
        *ptrNode = link;

    $$return;
}



void Parser::parse_line(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = { getTokenIndexByString("="), -1 };
    Expression::TNode* link = NULL;

    Assert_c(p < maxSize, "The file ended unexpectedly.");
    if (tokens[p].type == TOKEN_RETURN)
    {
        dataUnion.ivalue = OP_RETURN;
        safeINC(p);
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        if(tokens[p].type != TOKEN_SEMICOLON)
            parse_ORLogicExpr(&(*ptrNode)->link[0], p, *ptrNode);
        $$return;
    }
    
    if (tokens[p].type == TOKEN_VARIABLE_TYPE)
    {
        parse_var(&link, p, parent);
        if (tokens[p].type == TOKEN_SEMICOLON)
            $$return;

        Assert_parser(tokens[p].type == TOKEN_ASSIGMENT, "\'=\' was expected.");
        safeINC(p);
        dataUnion.ivalue = OP_ASSIGMENT;
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        parse_ORLogicExpr(&(*ptrNode)->link[1], p, *ptrNode);
        $$return;
    }

    parse_ORLogicExpr(ptrNode, p, parent);
    $$return;
}

void Parser::parse_branch(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;

    dataUnion.ivalue = OP_BRANCH;
    Assert_parser(tokens[p].type == TOKEN_IF, "\'if\' was expected.");
    safeINC(p);
    Assert_parser(tokens[p].type == TOKEN_BRACKET, "\'(\' was expected.");
    safeINC(p);

    *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, dataUnion, parent);

    parse_ORLogicExpr(&(*ptrNode)->link[0], p, *ptrNode);
    Assert_parser((*ptrNode)->link[0], "Parser couldn't parse the condition.");
    Assert_parser(tokens[p].type == TOKEN_BRACKET, "\')\' was expected.");
    safeINC(p);


    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'{\' was expected.");
    safeINC(p);
    parse_codeblock(&(*ptrNode)->link[1], p, *ptrNode);
    Assert_parser((*ptrNode)->link[1], "Parser couldn't parse the base block of code.");
    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'}\' was expected.");
    safeINC(p);

    if (tokens[p].type != TOKEN_ELSE)
        $$return;
    safeINC(p);

    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'{\' was expected.");
    safeINC(p);
    parse_codeblock(&(*ptrNode)->link[2], p, *ptrNode);
    Assert_parser((*ptrNode)->link[2], "Parser couldn't parse the base block of code.");
    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'}\' was expected.");
    safeINC(p);
}

void Parser::parse_loop(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;

    dataUnion.ivalue = OP_WHILE;
    Assert_parser(tokens[p].type == TOKEN_WHILE, "\'if\' was expected.");
    safeINC(p);

    Assert_parser(tokens[p].type == TOKEN_BRACKET, "\'(\' was expected.");
    safeINC(p);
    *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
    parse_ORLogicExpr(&(*ptrNode)->link[0], p, *ptrNode);
    Assert_parser((*ptrNode)->link[0], "Parser couldn't parse the condition.");
    Assert_parser(tokens[p].type == TOKEN_BRACKET, "\')\' was expected.");
    safeINC(p);


    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'{\' was expected.");
    safeINC(p);
    parse_codeblock(&(*ptrNode)->link[1], p, *ptrNode);
    Assert_parser((*ptrNode)->link[1], "Parser couldn't parse the base block of code.");
    Assert_parser(tokens[p].type == TOKEN_CURLY_BACKET, "\'}\' was expected.");
    safeINC(p);
}

void Parser::parse_var(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    Assert_c(p < maxSize, "The file ended unexpectedly.");

    Expression::TNode** link = ptrNode;

    Assert_c(tokens[p].type == TOKEN_VARIABLE_TYPE, "The \'var\' keyword was expected.");
    *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE_SPECIFICALOR, tokens[p].dataUnion, parent);
    safeINC(p);
    Assert_parser(tokens[p].type == TOKEN_NAME, "The variable name was expected.");
    (*ptrNode)->link[0] = createNode(NULL, NULL, NODE_TYPE_NAME, tokens[p].dataUnion, *ptrNode);
    safeINC(p);
    $$return;
}

void Parser::parse_ORLogicExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = { getTokenIndexByString("||"), -1 };
    Expression::TNode* link = NULL;

    Assert_c(p < maxSize, "The file ended unexpectedly.");
    parse_ANDLogicExpr(&link, p, parent);
    if (maxSize <= p)
    {
        (*ptrNode) = link;
        $$return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        dataUnion.ivalue = OP_OR;
        safeINC(p);
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        parse_ORLogicExpr(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$return;
}

void Parser::parse_ANDLogicExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = { getTokenIndexByString("&&"), -1 };
    Expression::TNode* link = NULL;

    Assert_c(p < maxSize, "The file ended unexpectedly.");
    parse_CMPLogicExpr(&link, p, parent);
    if (maxSize <= p)
    {
        (*ptrNode) = link;
        $$return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        dataUnion.ivalue = OP_AND;
        safeINC(p);
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        parse_ANDLogicExpr(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$return;
}

void Parser::parse_CMPLogicExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = {
        getTokenIndexByString(">"), getTokenIndexByString("<"),
        getTokenIndexByString(">="), getTokenIndexByString("<="),
        getTokenIndexByString("=="), getTokenIndexByString("!="),
        getTokenIndexByString("="), -1 };
    Expression::TNode* link = NULL;

    Assert_c(p < maxSize, "The file ended unexpectedly.");
    parse_additiveExpr(&link, p, parent);
    if (maxSize <= p)
    {
        (*ptrNode) = link;
        $$return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        dataUnion.ivalue = tokensTable[operations[op]].third;
        safeINC(p);
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        parse_additiveExpr(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$return;
}

void Parser::parse_additiveExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = { getTokenIndexByString("+"), getTokenIndexByString("-"), -1 };
    Expression::TNode* link = NULL;

    Assert_c(p < maxSize, "The file ended unexpectedly.");
    parse_multiplicativeExpr(&link, p, parent);
    if (maxSize <= p)
    {
        (*ptrNode) = link;
        $$return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        dataUnion.ivalue = tokensTable[operations[op]].third;
        safeINC(p);
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        parse_additiveExpr(&(*ptrNode)->link[1], p, *ptrNode);

    }
    else
        (*ptrNode) = link;

    $$return;
}

void Parser::parse_multiplicativeExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = { getTokenIndexByString("*"), getTokenIndexByString("^"), -1 };
    Expression::TNode* link = NULL;

    Assert_c(p < maxSize, "The file ended unexpectedly.");
    parse_fractionExpr(&link, p, parent);
    if (maxSize <= p)
    {
        (*ptrNode) = link;
        $$return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        dataUnion.ivalue = tokensTable[operations[op]].third;
        safeINC(p);
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        parse_multiplicativeExpr(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$return;
}

void Parser::parse_fractionExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    static const ui8 operations[] = { getTokenIndexByString("/"), -1 };
    Expression::TNode* link = NULL;

    Assert_c(p < maxSize, "The file ended unexpectedly.");
    parse_unaryOperator(&link, p, parent);
    if (maxSize <= p)
    {
        (*ptrNode) = link;
        $$return;
    }

    i32 op = findOperation(operations, tokens[p].dataUnion.ivalue);
    if (op != -1)
    {
        dataUnion.ivalue = OP_DIV;
        safeINC(p);
        *ptrNode = createNode(link, NULL, NODE_TYPE_OPERATION, dataUnion, parent);
        parse_unaryOperator(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$return;
}

void Parser::parse_unaryOperator(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    if (tokens[p].type == TOKEN_FUNCTION && tokens[p].dataUnion.ivalue == FUNC_NEG)
    {
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_FUNCTION, tokens[p].dataUnion, parent);
        safeINC(p);
        parse_operand(&(*ptrNode)->link[0], p, *ptrNode);
    }
    else
        parse_operand(ptrNode, p, *ptrNode);

    $$return;
}

void Parser::parse_operand(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    switch (tokens[p].type)
    {
    case TOKEN_NUMBER:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_NUMBER, tokens[p].dataUnion, parent);
        safeINC(p);
    break;
    case TOKEN_BRACKET:
        safeINC(p);
        parse_ORLogicExpr(ptrNode, p, parent);
        safeINC(p);
    break;
    default:
        parse_namedOperand(ptrNode, p, parent);
    break;
    }
    $$return;
}

void Parser::parse_namedOperand(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    standartVariableBlock;
    Expression::TNode* link = NULL;
    switch (tokens[p].type)
    {
    case TOKEN_NAME:
        link = createNode(NULL, NULL, NODE_TYPE_NAME, tokens[p].dataUnion, parent);
        safeINC(p);
        if (tokens[p].type == TOKEN_BRACKET && tokens[p].dataUnion.ivalue == 0) /// открывается скобка
        {
            safeINC(p);
            dataUnion.ivalue = 0;
            *ptrNode = createNode(link, NULL, NODE_TYPE_CUSTOM_FUNCTION, dataUnion, parent);
            parse_argumentsValue(&(*ptrNode)->link[1], p, *ptrNode);
            Assert_c(tokens[p].type == TOKEN_BRACKET);
            safeINC(p);
        }
        else
            *ptrNode = link;
    break;
    case TOKEN_FUNCTION:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_FUNCTION, tokens[p].dataUnion, parent);
        safeINC(p);
        Assert_c(tokens[p].type == TOKEN_BRACKET, "\'(\' was expected.");
        safeINC(p);
        if(tokens[p].type != TOKEN_BRACKET)
            parse_ORLogicExpr(&(*ptrNode)->link[0], p, *ptrNode);
        Assert_c(tokens[p].type == TOKEN_BRACKET, "\')\' was expected.");
        safeINC(p);
        break;
    break;
    default:
        $$$("Undefined token.");
        return;
    break;
    }
    $$return;
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
{$
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
            $$ return result;
        }
    }


    for (ui8 funcIndex = 0; funcIndex < FUNCTION_TABLE_SIZE; funcIndex++)
    {
        if (isStrStartFromSubStr(str, FUNCTION_NAMES_TABLE[funcIndex]))
        {
            result.type = TOKEN_FUNCTION;
            result.dataUnion.ivalue = funcIndex;
            str += strlen(FUNCTION_NAMES_TABLE[funcIndex]);
            $$ return result;
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

    $$ return result;
}

Expression::TNode* Parser::parse(C_string expression)
{$
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
    $$ return root;
}

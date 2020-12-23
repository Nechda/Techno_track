#include "Parser.h"
#include "CallStack.h"


//табличка со всеми токенами языка
const triple<C_string, Parser::LexemaType, OpType> Parser::tokensTable[] =
{
    { "(",    TOKEN_BRACKET,                OP_UNDEFINED},
    { ")",    TOKEN_BRACKET,                OP_UNDEFINED},
    { "{",    TOKEN_CURLY_BACKET,           OP_UNDEFINED},
    { "}",    TOKEN_CURLY_BACKET,           OP_UNDEFINED},
    { ";",    TOKEN_SEMICOLON,              OP_SEMICOLON},
    { "+",    TOKEN_ARITHMETIC_OPERATION,   OP_SUM},
    { "-",    TOKEN_ARITHMETIC_OPERATION,   OP_SUB},
    { "*",    TOKEN_ARITHMETIC_OPERATION,   OP_MUL},
    { "/",    TOKEN_ARITHMETIC_OPERATION,   OP_DIV},
    { "^",    TOKEN_ARITHMETIC_OPERATION,   OP_POW},
    { "var",  TOKEN_VARIABLE_TYPE,          OP_UNDEFINED},
    { "if",   TOKEN_IF,                     OP_BRANCH},
    { "else", TOKEN_ELSE,                   OP_UNDEFINED},
    { "||",   TOKEN_OR,                     OP_OR},
    { "&&",   TOKEN_AND,                    OP_AND},
    { ">",    TOKEN_LOGIC_OPERATION,        OP_GAIN},
    { "<",    TOKEN_LOGIC_OPERATION,        OP_LESS},
    { ">=",   TOKEN_LOGIC_OPERATION,        OP_GAIN_OR_EQUAL},
    { "<=",   TOKEN_LOGIC_OPERATION,        OP_LESS_OR_EQUAL},
    { "==",   TOKEN_LOGIC_OPERATION,        OP_EQUAL},
    { "!=",   TOKEN_LOGIC_OPERATION,        OP_NEQUAL},
    { "=",    TOKEN_ASSIGMENT,              OP_ASSIGMENT},
    { "def",  TOKEN_DEF,                    OP_DEF},
    { ",",    TOKEN_COMMA,                  OP_COMMA},
    { "$",    TOKEN_UNDEFINED,              OP_DOLLAR} /// <--- этот токен только для объеднинения функций

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
$$
}


/*
    Fl - File
    Fu - Function
    fN - Function name
    Ar - Arguments of function
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

    LE1 -> LE2 || LE2 | LE2
    LE2 -> LE3 && LE3 | LE3
    LE3 -> E < E | E > E | E <= E | E >= E | E == E | E != E | E = E

    E -> T + E | T - E | T
    T -> D * T | D ^ T | D
    D -> F / F | F
    F -> N     | (LE1)   | -(LE1)  | -F
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
    Expression::TNode* link = NULL;
    parse_function(&link, p, parent);

    if(!link)
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

    unionData.ivalue = OP_DEF;
    bool isErrorOccur = 0;
    isErrorOccur |= !(tokens[p].type == TOKEN_DEF);
    checkErrors();
    safeINC(p);

    isErrorOccur |= !(tokens[p].type == TOKEN_VARIABLE);
    checkErrors();
    safeINC(p);
    *ptrNode = createNode(NULL, NULL, NODE_TYPE_OPERATION, unionData, parent);
    unionData = tokens[p].dataUnion;
    (*ptrNode)->link[0] = createNode(NULL, NULL, NODE_TYPE_VARIABLE, unionData, *ptrNode);
    
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
{$
    Expression::TNode* link = NULL;
    UnionData unionData;

    switch (tokens[p].type)
    {
    case TOKEN_VARIABLE_TYPE:
        link = createNode(NULL, NULL, NODE_TYPE_VARIABLE_SPECIFICALOR, tokens[p].dataUnion, parent);
        p++;
        parse_fact(&link->link[0], p, link);
        break;
    default:
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


void Parser::parse_general(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    static const ui8 operations[] = { getTokenIndexByString(";"), -1};
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
{$
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
    if (op != -1)
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

void Parser::parse_var(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    switch (tokens[p].type)
    {
    case TOKEN_VARIABLE_TYPE:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE_SPECIFICALOR, tokens[p].dataUnion, parent);
        p++;
        parse_fact(&(*ptrNode)->link[0], p, *ptrNode);
        break;
    case TOKEN_VARIABLE:
        *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE, tokens[p].dataUnion, parent);
        p++;
        break;
    default:
        Assert_c("Undefined token type.");
        $$$("Undefined token type.");
        return;
    }
    $$
}


void Parser::parse_logicExpr1(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    static const ui8 operations[] = { getTokenIndexByString("||"), -1};
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
        parse_logicExpr2(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
    return;
}

void Parser::parse_logicExpr2(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    static const ui8 operations[] = { getTokenIndexByString("&&"), -1};
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
        parse_logicExpr3(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
    return;
}

void Parser::parse_logicExpr3(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    static const ui8 operations[] = {
        getTokenIndexByString(">"),getTokenIndexByString("<"),
        getTokenIndexByString(">="),getTokenIndexByString(">="),
        getTokenIndexByString("=="),getTokenIndexByString("!="),
        getTokenIndexByString("="),-1};
    Expression::TNode* link = NULL;

    parse_expr(&link, p, parent);
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


void Parser::parse_expr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
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
{$
    static const ui8 operations[] = {getTokenIndexByString("*"), getTokenIndexByString("^"), -1};
    Expression::TNode* link = NULL;

    parse_divider(&link, p, parent);
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
        parse_term(&(*ptrNode)->link[1], p, *ptrNode);
    }
    else
        (*ptrNode) = link;

    $$
    return;
}

void Parser::parse_divider(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent)
{$
    static const ui8 operations[] = {getTokenIndexByString("/"), -1};
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
{$
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
        case TOKEN_VARIABLE:
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_VARIABLE, tokens[p].dataUnion, parent);
            p++;
            break;
        case TOKEN_NUMBER:
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_NUMBER, tokens[p].dataUnion, parent);
            p++;
            break;
        case TOKEN_FUNCTION:
            *ptrNode = createNode(NULL, NULL, NODE_TYPE_FUNCTION, tokens[p].dataUnion, parent);
            if (tokens[p + 1].type == TOKEN_BRACKET)
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


/**
\brief Таблица перестановки 256 энементов для алгоритма хеширования Пирсона
*/
static const ui8 T[256] = {
    249, 232, 89, 20, 244, 97, 50, 114, 220, 107, 86, 150, 67, 233, 42, 226,
    209, 3, 206, 74, 207, 180, 85, 216, 21, 191, 246, 82, 137, 186, 128, 40,
    172, 15, 96, 93, 152, 60, 240, 95, 122, 2, 164, 33, 112, 17, 201, 129,
    22, 248, 225, 132, 76, 163, 127, 139, 118, 57, 136, 8, 37, 245, 195, 16,
    43, 87, 69, 0, 39, 188, 254, 130, 251, 213, 243, 222, 78, 223, 6, 228,
    231, 211, 106, 119, 124, 174, 155, 14, 189, 29, 101, 113, 70, 196, 18, 173,
    35, 167, 229, 92, 239, 157, 83, 28, 25, 212, 215, 237, 203, 62, 10, 156,
    160, 63, 59, 9, 79, 44, 141, 47, 34, 252, 158, 90, 64, 68, 27, 170,
    56, 49, 108, 146, 5, 236, 100, 55, 26, 178, 175, 241, 65, 110, 54, 159,
    147, 205, 135, 224, 198, 61, 120, 1, 154, 208, 7, 126, 138, 32, 161, 53,
    165, 71, 148, 73, 13, 94, 11, 84, 38, 104, 77, 45, 81, 131, 193, 255,
    234, 88, 217, 179, 4, 116, 219, 145, 168, 75, 171, 204, 192, 140, 166, 185,
    30, 218, 151, 48, 24, 176, 80, 143, 149, 153, 51, 210, 121, 58, 235, 200,
    125, 103, 197, 177, 184, 221, 181, 52, 19, 230, 242, 134, 109, 123, 31, 187,
    12, 111, 23, 238, 253, 36, 98, 66, 247, 117, 227, 133, 72, 169, 102, 41,
    105, 46, 190, 214, 194, 250, 199, 91, 202, 162, 142, 182, 183, 99, 144, 115
};

/**
\brief   Генерация 64 битного хеша по алгоритму Пирсона
\param   [in]   data   Указатель на массив, по которому строится хеш
\param   [in]   len    Размер передаваемого массива
\return  Возвращается 64 битных хеш, сгенерированный по алгоритму Пирсона
*/
static ui64 getHash(const ui8* data, ui32 len)
{
    union
    {
        ui64 hash;
        ui8 hPtr[8];
    }Hash;
    ui8 h = 0;
    for (int j = 0; j < 8; j++)
    {
        h = T[(data[0] + j) % 256];
        for (int i = 0; i < len; i++)
            h = T[h^data[i]];
        Hash.hPtr[j] = h;
    }
    return Hash.hash;
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
        if (isEqualTo(str, tokensTable[tokenIndex].first))
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
        if (isEqualTo(str, FUNCTION_NAMES_TABLE[funcIndex]))
        {
            result.type = TOKEN_FUNCTION;
            result.dataUnion.ivalue = funcIndex;
            str += strlen(FUNCTION_NAMES_TABLE[funcIndex]);
            $$
            return result;
        }
    }
    

    sscanf(str, "%[^ ;=+-*\\/()^{},!<>]%*c", buff);
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
        result.type = TOKEN_VARIABLE;
        result.dataUnion.ivalue = getHash((ui8*)buff, strlen(buff));
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
        if (it->type == TOKEN_ARITHMETIC_OPERATION && it->dataUnion.ivalue == '-' && wasSpaceOrBracket)
        {
            it->type = TOKEN_FUNCTION;
            it->dataUnion.ivalue = FUNC_NEG;
        }
        wasSpaceOrBracket = it->dataUnion.ivalue == '(';
    }

    #define PARSER_DEBUG

    #ifdef PARSER_DEBUG
    ui32 counter = 0;
    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        printf("[%d] data:",counter);
        switch (it->type)
        {
            case TOKEN_NUMBER:
                printf("%lf",it->dataUnion.dvalue);
                break;
            case TOKEN_VARIABLE:
                printf("VARIABLE_HASH = 0x%X",it->dataUnion.ivalue);
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

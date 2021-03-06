#pragma once
#include "ExprTree.h"
#include "Types.h"
#include <vector>

//#define PARSER_DEBUG


template <typename T1, typename T2, typename T3>
struct triple
{
    T1 first;
    T2 second;
    T3 third;
};

class Parser
{
    private:
        enum LexemaType
        {
            TOKEN_UNDEFINED,
            TOKEN_NUMBER,
            TOKEN_ARITHMETIC_OPERATION,
            TOKEN_BRACKET,
            TOKEN_CURLY_BACKET,
            TOKEN_SEMICOLON,
            TOKEN_ASSIGMENT,
            TOKEN_NAME,
            TOKEN_FUNCTION,
            TOKEN_VARIABLE_TYPE,
            TOKEN_IF,
            TOKEN_ELSE,
            TOKEN_OR,
            TOKEN_AND,
            TOKEN_LOGIC_OPERATION,
            TOKEN_DEF,
            TOKEN_COMMA,
            TOKEN_RETURN,
            TOKEN_WHILE
        };

        static const triple<C_string, LexemaType, OpType> tokensTable[];
        static const ui8 TOKENS_TABLE_SIZE;
        ui8 getTokenIndexByString(C_string str);
        typedef Expression::TNode* TPNode;

        struct Token
        {
            LexemaType type;
            UnionData dataUnion;
        };
        std::vector<Token> tokens;

        struct ParserData
        {
            TPNode& ptrNode;
            ui32& p;
            TPNode& parent;
        };

        void parse_file(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_function(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_arguments(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_argumentsValue(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_codeblock(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_line(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_branch(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_loop(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_var(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_ORLogicExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_ANDLogicExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_CMPLogicExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_additiveExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_multiplicativeExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_fractionExpr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_unaryOperator(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_operand(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_namedOperand(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);

        Token getNextToken(C_string& str);
    public:
        Parser() {};
        ~Parser();
        Expression::TNode* parse(C_string expression);
};
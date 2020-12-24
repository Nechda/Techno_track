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
            TOKEN_RETURN
        };

        static const triple<C_string, LexemaType, OpType> tokensTable[];
        static const ui8 TOKENS_TABLE_SIZE;
        ui8 getTokenIndexByString(C_string str);

        struct Token
        {
            LexemaType type;
            UnionData dataUnion;
        };

        std::vector<Token> tokens;

        void parse_file(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_function(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_arguments(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_argumentsValue(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_general(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_line(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_branch(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_var(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        
        void parse_logicExpr1(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_logicExpr2(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_logicExpr3(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        
        void parse_expr(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_term(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_divider(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        void parse_fact(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent);
        Token getNextToken(C_string& str);
    public:
        Parser() {};
        ~Parser();
        Expression::TNode* parse(C_string expression);
};
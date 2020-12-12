#pragma once
#include "ExprTree.h"
#include "Types.h"
#include <vector>

//#define PARSER_DEBUG


class Parser
{
    private:
        enum LexemaType
        {
            LEX_UNDEFINED,
            LEX_NUMBER,
            LEX_OPERATION,
            LEX_BRACKET,
            LEX_VARIABLE,
            LEX_FUNCTION
        };

        struct Token
        {
            LexemaType type;
            union
            {
                char symbol;
                double value;
            }ptrToData;
        };

        std::vector<Token> tokens;

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
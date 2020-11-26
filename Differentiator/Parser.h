#pragma once
#include "ExprTree.h"
#include "Types.h"
#include <vector>

class Parser
{
    private:
        enum LexemaType
        {
            LEX_UNDEFINED,
            LEX_NUMBER,
            LEX_OPERATION,
            LEX_BRACKET,
            LEX_VARIABLE
        };

        enum OperationType
        {
            OP_SUM,
            OP_SUB,
            OP_MUL,
            OP_DIV
        };

        struct Token
        {
            LexemaType type;
            union
            {
                char symbol;
                ui32 ivalue;
                double dvalue;
            }data;
        };

        std::vector<Token> tokens;
        ui32 isOperation(ui8 opType, ui32 start, ui32 end);


        void parse_expr(Expression::TNode** ptrNode, ui32 start, ui32 end);
        void parse_term(Expression::TNode** ptrNode, ui32 start, ui32 end);
        void parse_fact(Expression::TNode** ptrNode, ui32 start, ui32 end);
        Token getNextToken(C_string& str);
    public:
        Parser() {};
        ~Parser();
        Expression::TNode* parse(C_string expression);
};
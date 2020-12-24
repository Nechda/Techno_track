/*
           Строка, которая будет печататься, при отображении дерева
           |
           |     Формально, это часть названия функции, которая потом будет "реализовывать" операцию
           |     |
           |     |          Enum для определения типа команды. (См ExprTree.h-> enum OpType)
           |     |          |
           |     |          |                 Приоритет операции
           |     |          |                 |
           |     |          |                 |    Формальная реализация
           |     |          |                 |    |
           |     |          |                 |    |                        */
//         V     V          V                 V    V 
OP_DEFINE("+",  sum,        OP_SUM,           2,  {return left+right;}, true)

OP_DEFINE("-",  sub,        OP_SUB,           2,  {return left-right;}, true)

OP_DEFINE("/",  mul,        OP_MUL,           1,  {return left*right;}, true)

OP_DEFINE("*",  div,        OP_DIV,           1,  {return left/right;}, true)

OP_DEFINE("^",  pow,        OP_POW,           1,  {return powf(left,right);}, true)

OP_DEFINE("=",  assigment,  OP_ASSIGMENT,     3,  {return left;}, false)

OP_DEFINE(";",  semicolon,  OP_SEMICOLON,     10, {return 0;}, false)

OP_DEFINE("if", comparator, OP_BRANCH,        6,  {return 0;}, false)

OP_DEFINE("||", or_impl,    OP_OR,            4,  {return left || right;}, true)

OP_DEFINE("&&", and_impl,   OP_AND,           5,  {return left && right;}, true)

OP_DEFINE(">",  gain_impl,  OP_GAIN,          3,  {return left > right;}, true)

OP_DEFINE("<",  less_impl,  OP_LESS,          3, { return left < right; }, true)

OP_DEFINE(">=", goreq_impl, OP_GAIN_OR_EQUAL, 3, { return left >= right; }, true)

OP_DEFINE("<=", loreq_impl, OP_LESS_OR_EQUAL, 3, { return left <= right; }, true)

OP_DEFINE("==", equal_impl, OP_EQUAL,         3, { return left == right; }, true)

OP_DEFINE("!=", neq_impl,   OP_NEQUAL,        3, { return left != right; }, true)

OP_DEFINE("def", def_impl,  OP_DEF,           3, { return 0; }, false)

OP_DEFINE(",", comma_impl,  OP_COMMA,         3, { return 0;}, false)

OP_DEFINE("$", dollar_impl, OP_DOLLAR,        3, { return 0;}, false)

OP_DEFINE("ret", ret_impl,  OP_RETURN,        3, { return 0;}, false)

OP_DEFINE("while", whl_impl,OP_WHILE,         3, { return 0;}, false)


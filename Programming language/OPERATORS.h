
/*
           Строка, которая будет печататься, при отображении дерева
           |
           |     Формально, это часть названия функции, которая потом будет "реализовывать" операцию
           |     |
           |     |          Enum для определения типа команды. (См ExprTree.h-> enum OpType)
           |     |          |
           |     |          |                 Приоритет операции (использовался для построения теха,
           |     |          |                 |    сейчас все приоритеты зашиты в рекурсивном спуске)
           |     |          |                 |
           |     |          |                 |    Формальная реализация для вычисления дерева
           |     |          |                 |    |
           |     |          |                 |    |                    Можно ли на этапе компиляции вычислить эту функцию
           |     |          |                 |    |                    |
           |     |          |                 |    |                    |
           |     |          |                 |    |                    |               Код, транслирующий дерево в ASM процессора
           |     |          |                 |    |                    |                       |                                                */
//         V     V          V                 V    V                    V                       |
OP_DEFINE("+",  sum,        OP_SUM,           2,  {return left+right;}, true,//                 |
{                                                                            //                 |        
    preparationForOperatorWith2Operands(node, asmInfo);                      //   <-------------+
    throwLineForOperatorWith2Operands("add", asmInfo);                       //
}                                                                            //
)                                                                            //

OP_DEFINE("-",  sub,        OP_SUB,           2,  {return left-right;}, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForOperatorWith2Operands("sub", asmInfo);
}
)

OP_DEFINE("/",  mul,        OP_MUL,           1,  {return left*right;}, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForOperatorWith2Operands("mul", asmInfo);
}
)

OP_DEFINE("*",  div,        OP_DIV,           1,  {return left/right;}, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForOperatorWith2Operands("div", asmInfo);
}
)

OP_DEFINE("^",  pow,        OP_POW,           1,  {return powf(left,right);}, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForOperatorWith2Operands("pow", asmInfo);
}
)

OP_DEFINE("=",  assigment,  OP_ASSIGMENT,     3,  {return left;}, false,
{
    if (node->link[0]->ptrToData->type == NODE_TYPE_VARIABLE_SPECIFICALOR)
    {
        genAsmByTree(node->link[0], asmInfo); // этот вызов позволит записать в таблицу новую переменную
        variableNameHash = node->link[0]->link[0]->ptrToData->dataUnion.ivalue;
    }
    else
        variableNameHash = node->link[0]->ptrToData->dataUnion.ivalue;

    genAsmByTree(node->link[1], asmInfo);

    if (!asmInfo.tableAdrVariables.count(variableNameHash))
    {
        Assert_c("Undefined variable");
        $$$("Undefined variable");
        return;
    }

    throwLineForAccessToVariable(variableNameHash, OperationOnVar::VAR_OP_SET, asmInfo);

    //pushLine("pop [ebp + %d]", asmInfo.tableAdrVariables[variableNameHash] * SIZE_OPERANDS);
}
)

OP_DEFINE(";",  semicolon,  OP_SEMICOLON,     10, {return 0;}, false,
{
    genAsmByTree(node->link[0], asmInfo);
    genAsmByTree(node->link[1], asmInfo);
}
)

OP_DEFINE("if", comparator, OP_BRANCH,        6,  {return 0;}, false,
{
    genAsmByTree(node->link[0], asmInfo);
    pushLine("pop eax");
    pushLine("cmp eax, 0");
    pushLine("jne if_%X", asmInfo.labelCountIfOperator);
    pushLine("je else_%X", asmInfo.labelCountIfOperator);
    pushLine("if_%X:", asmInfo.labelCountIfOperator);
    genAsmByTree(node->link[1], asmInfo);
    pushLine("jmp endif_%X", asmInfo.labelCountIfOperator);
    pushLine("else_%X:", asmInfo.labelCountIfOperator);
    genAsmByTree(node->link[2], asmInfo);
    pushLine("endif_%X:", asmInfo.labelCountIfOperator);
    asmInfo.labelCountIfOperator++;
}
)



OP_DEFINE("||", or_impl,    OP_OR,            4,  {return left || right;}, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForOperatorWith2Operands("or", asmInfo);
}
)

OP_DEFINE("&&", and_impl,   OP_AND,           5,  {return left && right;}, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForOperatorWith2Operands("and", asmInfo);
}
)

OP_DEFINE(">",  gain_impl,  OP_GAIN,          3,  {return left > right;}, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForСomparisonOperators("ja", asmInfo);
}
)

OP_DEFINE("<",  less_impl,  OP_LESS,          3, { return left < right; }, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForСomparisonOperators("jb", asmInfo);
}
)

OP_DEFINE(">=", goreq_impl, OP_GAIN_OR_EQUAL, 3, { return left >= right; }, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForСomparisonOperators("jae", asmInfo);
}
)

OP_DEFINE("<=", loreq_impl, OP_LESS_OR_EQUAL, 3, { return left <= right; }, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForСomparisonOperators("jbe", asmInfo);
}
)

OP_DEFINE("==", equal_impl, OP_EQUAL,         3, { return left == right; }, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForСomparisonOperators("je", asmInfo);
}
)

OP_DEFINE("!=", neq_impl,   OP_NEQUAL,        3, { return left != right; }, true,
{
    preparationForOperatorWith2Operands(node, asmInfo);
    throwLineForСomparisonOperators("jne", asmInfo);
}
)

OP_DEFINE("def", def_impl,  OP_DEF,           3, { return 0; }, false,
{
    genAsmByTree(node->link[1], asmInfo); // этот вызов пропарсит переменные, которые передаются в функцию
    genAsmByTree(node->link[2], asmInfo); // этот вызов начнет парсинг кода самой функции
}
)

OP_DEFINE(",", comma_impl,  OP_COMMA,         3, { return 0;}, false,
{
    genAsmByTree(node->link[0], asmInfo);
    genAsmByTree(node->link[1], asmInfo);
}
)

OP_DEFINE("$", dollar_impl, OP_DOLLAR,        3, { return 0;}, false,
{

}
)

OP_DEFINE("ret", ret_impl,  OP_RETURN,        3, { return 0;}, false,
{
    genAsmByTree(node->link[0], asmInfo);
    pushLine("pop eax");
    pushLine("mov esp, ebp");
    pushLine("ret");
}
)

OP_DEFINE("while", whl_impl,OP_WHILE,         3, { return 0;}, false,
{
    pushLine("loop_%X:",asmInfo.labelCountLoopOperator);
    genAsmByTree(node->link[0], asmInfo);
    pushLine("pop ecx");
    pushLine("cmp ecx, 0");
    pushLine("je end_loop_%X", asmInfo.labelCountLoopOperator);
    genAsmByTree(node->link[1], asmInfo);
    pushLine("jmp loop_%X", asmInfo.labelCountLoopOperator);
    pushLine("end_loop_%X:", asmInfo.labelCountLoopOperator);
    asmInfo.labelCountLoopOperator++;
}
)
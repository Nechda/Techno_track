#include "ExprTree.h"
#include <stdarg.h>
#include <vector>

using std::vector;


struct
{
    Stream stream;
    bool isValid() { return stream; }
}filePuller;
static void pushLine(const C_string format, ...)
{
    va_list argList;
    va_start(argList, format);
    vfprintf(filePuller.stream, format, argList);
    va_end(argList);
    fprintf(filePuller.stream,"\n");
}

/*
    Немного констант, задающие работу процессора
*/
const ui32 SIZE_OPERANDS = sizeof(float);

/*
    скорее всего, потребуется функция, которая будет выдавать строку
    в 10 + 27 ричной системе счисления, это требуется для того, чтобы
    можно было по максимому использовать имена меток
*/


struct DataForAssembler
{
    map<Hash, ui32> tableAdrVariables;
    ui32 labelCountComparison;
    ui32 labelCountIfOperator;
    ui32 labelCountLoopOperator;
    CompilatorError& errorCode;
    const map<Hash, Expression::TNode*>& functionsTable;
};

static void genAsmByTree(Expression::TNode* node, DataForAssembler& asmInfo);

static void preparationForOperatorWith2Operands(Expression::TNode* node, DataForAssembler& asmInfo)
{
    genAsmByTree(node->link[1], asmInfo);
    genAsmByTree(node->link[0], asmInfo);
}

static void throwLineForOperatorWith2Operands(const C_string strOperationCommand, DataForAssembler& asmInfo)
{
    pushLine("pop eax");
    pushLine("pop ebx");
    pushLine("%s eax, ebx", strOperationCommand);
    pushLine("push eax");
}

static void throwLineForСomparisonOperators(const C_string strJumpCommand, DataForAssembler& asmInfo)
{
    pushLine("pop eax");
    pushLine("pop ebx");
    pushLine("cmp eax, ebx");
    pushLine("%s T_%s_%X", strJumpCommand, strJumpCommand, asmInfo.labelCountComparison);
    pushLine("push 0.0");
    pushLine("jmp end_%s_%X", strJumpCommand, asmInfo.labelCountComparison);
    pushLine("T_%s_%X:", strJumpCommand, asmInfo.labelCountComparison);
    pushLine("push 1.0");
    pushLine("end_%s_%X:", strJumpCommand, asmInfo.labelCountComparison);
    asmInfo.labelCountComparison++;
}

static void throwLineForCallFunctions(Expression::TNode* node, Hash functionNameHash, DataForAssembler& asmInfo)
{
    pushLine("push ebp");
    pushLine("mov edi, esp");
    pushLine("push 0xADD");
    if(node)genAsmByTree(node->link[1], asmInfo);
    pushLine("mov ebp, edi");
    pushLine("call func_%X", functionNameHash);
    pushLine("pop ebp");
}

enum class OperationOnVar
{
    VAR_OP_SET,
    VAR_OP_GET
};

static void throwLineForAccessToVariable(Hash variableNameHash, OperationOnVar opOnVar, DataForAssembler& asmInfo)
{
    pushLine("push ebp");
    pushLine("add ebp, %d", asmInfo.tableAdrVariables[variableNameHash] * SIZE_OPERANDS);
    pushLine("mov esi, ebp");
    pushLine("pop ebp");
    if(opOnVar == OperationOnVar::VAR_OP_GET)
        pushLine("push [esi]");
    if(opOnVar == OperationOnVar::VAR_OP_SET)
        pushLine("pop [esi]");
}

static ui32 countCertainNodeInTree(Expression::TNode* node, NodeType type)
{
    if (!node)
        return 0;
    ui32 result = 0;
    if (node->ptrToData->type == type)
        result = 1;
    for (ui8 i = 0; i < TREE_CHILD_NUMBER; i++)
        result += countCertainNodeInTree(node->link[i], type);
    return result;
}




static void genAsmByTree(Expression::TNode* node, DataForAssembler& asmInfo)
{$
    if (!filePuller.isValid() || !node || asmInfo.errorCode)
    {
        $$ return;
    }

    Hash variableNameHash = 0;
    Hash functionNameHash = 0;
    if(node->ptrToData->type == NODE_TYPE_OPERATION)
        switch (node->ptrToData->dataUnion.ivalue)
        {
            #define OP_DEFINE(string, name, enumName, priority, implentation, canUseInConstantSimplify, asmCodeTranslator)\
            case enumName:\
                asmCodeTranslator\
            break;
            #include "OPERATORS.h"
            #undef OP_DEFINE
            default:
            break;
        }

    if (node->ptrToData->type == NODE_TYPE_FUNCTION)
        switch (node->ptrToData->dataUnion.ivalue)
        {
            #define FUNC_DEFINE(name, enumName, implentation, canUseInConstantSimplify, asmCodeTranslator)\
            case enumName:\
                asmCodeTranslator\
            break;
            #include "FUNCTIONS.h"
            #undef FUNC_DEFINE
            default:
            break;
        }

    if (node->ptrToData->type == NODE_TYPE_CUSTOM_FUNCTION)
    {
        functionNameHash = node->link[0]->ptrToData->dataUnion.ivalue;
        if (!asmInfo.functionsTable.count(functionNameHash))
        {
            asmInfo.errorCode = ERR_UNDEFINED_FUNCTION;
            Assert_c("Undefined function");
            return;
        }
        throwLineForCallFunctions(node, functionNameHash, asmInfo);
        pushLine("push eax");
    }

    if (node->ptrToData->type == NODE_TYPE_NAME)
    {
        variableNameHash = node->ptrToData->dataUnion.ivalue;
        if (!asmInfo.tableAdrVariables.count(variableNameHash))
        {
            asmInfo.errorCode = ERR_UNDEFINED_VARIABLE;
            Assert_c("Undefined variable");
            $$$("Undefined variable");
            return;
        }
        
        throwLineForAccessToVariable(variableNameHash, OperationOnVar::VAR_OP_GET, asmInfo);

        //pushLine("push [ebp + %d]", asmInfo.tableAdrVariables[variableNameHash] * SIZE_OPERANDS);

    }

    if (node->ptrToData->type == NODE_TYPE_NUMBER)
        pushLine("push %f", static_cast<float>(node->ptrToData->dataUnion.dvalue));

    if (node->ptrToData->type == NODE_TYPE_VARIABLE_SPECIFICALOR)
    {
        variableNameHash = node->link[0]->ptrToData->dataUnion.ivalue;
        if (asmInfo.tableAdrVariables.count(variableNameHash))
        {
            asmInfo.errorCode = ERR_REDIFINITION_VARIABLE;
            Assert_c("Variable redefinition");
            $$$("Variable redefinition");
            return;
        }
        asmInfo.tableAdrVariables[variableNameHash] = asmInfo.tableAdrVariables.size();
    }
    $$ return;
}

void Expression::compile(const C_string filename)
{$
    //stage 1 генерация таблицы функций
    genFunctionTable();

    //stage 2 подсчитываем количество локальных переменных для каждой функции
    //note: считать будем только переменные, которые находятся в теле функции
    vector<ui32> nVariables;
    for(auto it : functionsTable)
        nVariables.push_back(countCertainNodeInTree(it.second->link[2], NODE_TYPE_VARIABLE_SPECIFICALOR));

    //stage 3 рекурсивно парсим деревья в файл
    filePuller.stream = fopen(filename, "w");
    errorCode = STATUS_OK;

    DataForAssembler asmInfo = {
        map<Hash, ui32>(),
        0,
        0,
        0,
        errorCode,
        functionsTable
    };

    if (functionsTable.count(entryPointHash))
    {
        pushLine("fpmon");
        throwLineForCallFunctions(NULL, entryPointHash, asmInfo);
        pushLine("hlt");
    }
    else
    {
        errorCode = ERR_NO_ENTRY_POINT_FUNCTION;
        Assert_c("There is no entry point. By default use main().");
        fclose(filePuller.stream);
        $$ return;
    }


    auto vit = nVariables.begin();
    for (auto it : functionsTable)
    {
        pushLine("func_%X:", it.first);
        if (it.first == entryPointHash && false)
            pushLine("mov ebp, esp");
        else
        {
            pushLine("pop [ebp]");
            pushLine("add ebp, 4");
            pushLine("add esp, %d", *vit * SIZE_OPERANDS);
        }
        genAsmByTree(it.second, asmInfo);
        if (errorCode)
        {
            fclose(filePuller.stream);
            $$ return;
        }
        pushLine("mov esp, ebp");
        pushLine("ret");
        asmInfo.tableAdrVariables.clear();
        vit++;
    }

    fclose(filePuller.stream);
    $$ return;
}
#include "Asm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>




/**
\brief  Функция полностью сичтывает файл
\param  [in]      filename  Имя считываемого файла
\param  [in,out]  outString Указатель на считанную строку
\param  [in]      readBytesPtr  Указатель на unsigned, в котором будет храниться количество считанных байтов
\return В случае успеха возвращается количество прочитанных байт.
        Если произошла ошибка, то возвращается константа ASM_ERROR_CODE.
*/
int readFullFile(const char* filename, char** outString)
{
    Assert_c(filename);
    Assert_c(outString);
    if (!filename || !outString)
        return ASM_ERROR_CODE;

    FILE* inputFile = fopen(filename, "rb");
    Assert_c(inputFile);
    if (!inputFile)
        return ASM_ERROR_CODE;
    if (ferror(inputFile))
        return ASM_ERROR_CODE;

    fseek(inputFile, 0, SEEK_END);
    long fsize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    char* string = (char*)calloc(fsize + 1, sizeof(char));
    Assert_c(string);
    if (!string)
        return ASM_ERROR_CODE;

    unsigned nReadBytes = fread(string, sizeof(char), fsize, inputFile);
    fclose(inputFile);
    string[fsize] = 0;

    *outString = string;

    return nReadBytes;
}



/*
\brief Структура, описывающая минимальную единицу языка
*/
struct Lexema
{
    char command[10];
    Mcode machineCode;
    char* validFirstOperand;
    char* validSecondOperand;
};




/*
\brief Таблица с описанием всех команд, которые поддерживает компилятор
*/
Lexema Table[] = {
    #define DEF(name, machineCode, validStrOperand_1, validStrOperand_2, code) \
    {#name, machineCode, validStrOperand_1, validStrOperand_2},
    #include "Extend.h"
    #undef DEF
};


/*
\brief Таблица с описанием всех регистов, которые поддерживает компилятор
*/
const Lexema Registers[] =
{
    {"eax",     1},
    {"ebx",     2},
    {"ecx",     3},
    {"edx",     4},
    {"esi",     5},
    {"edi",     6},
    {"esp",     7},
    {"ebp",     8},
    {"eip",     9},
    {"eflags", 10},
    {"ecs",    11},
    {"eds",    12},
    {"ess",    13}
};


/*
\brief Описание констант, задающих размер таблиц
@{
*/
const ui32 COMMAND_TABLE_SIZE = sizeof(Table) / sizeof(Lexema);; ///< всего 18 базовых команд, которые не реализуются с помощью макрасов 
const ui32 REGISTER_TABLE_SIZE = sizeof(Registers) / sizeof(Lexema);
/*
@}
*/


/**
\brief  Функция переводит все буквы в строке в нижний регистр
\param  [in]  str  Входящая строка
*/
inline void toLowerStr(char* str)
{
    if (!str)
        return;
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }
}

void prepareCompilatorsTable()
{
    for (int i = 0; i < COMMAND_TABLE_SIZE; i++)
    {
        if(!islower(Table[i].command[0]))
            toLowerStr(Table[i].command);
    }
}


/*
\brief  Функция определяет количество операндов по машинному коду
\param  [in]  machCode  Машинный код
\return Количество операндов в данной команде
*/
inline ui8 getNumberOperands(Mcode marchCode)
{
    return marchCode & 0x3;
}

/*
\brief  Функция присваивает машинной команде тип операнда
\param  [in]  ptrMarchCode  Указатель на машинную команду
\param  [in]  opIndex       Номер операнда
\param  [in]  type          Тип операнда
*/
inline void setOperandType(Mcode* ptrMarchCode, ui8 opIndex, OperandType type)
{
    *ptrMarchCode &= ~(0b11 << (2 * opIndex + 2));
    *ptrMarchCode |= ((ui8)type & 0b11) << (2 * opIndex + 2);
}


/*
\brief  Функция определяет тип операнда по машинной команде
\param  [in]  marchCode  Машинная команда
\param  [in]  opIndex    Номер операнда
\return Тип операнда, имеющий индекс opIndex
*/
inline OperandType getOperandType(Mcode marchCode, ui8 opIndex)
{
    ui8 shift = (2 * opIndex + 2);
    marchCode >>= shift;
    marchCode &= 0b11;
    return (OperandType)(marchCode);
}

/*
\brief  Функция определяет номер команды по машинному коду, игнорируя первые 6 битов
\param  [in]  marchCode  Машинная команда
\return Машинный код, без информации о типах и количестве операндов
*/
inline Mcode getPureMachCode(Mcode machCode)
{
    return machCode & (~0b111100);
}

/*
\brief  Функция возвращает поясняющую строку, по коду ошибки компилятора\дизасемблера
\param  [in]  errorCode  Код ошибки
\return Возвращается строка, поясняющее код ошибки
*/
C_string getStringByErrorCode(AsmError errorCode)
{
    switch (errorCode)
    {
        case ASM_OK:
            return "Ok";
            break;
        case ASM_ERROR_INVALID_INPUT_DATA:
            return "Due to compilating occur error ralated with invalid input data";
            break;
        case ASM_ERROR_OUT_OF_MEMORY:
            return "Due to compilating occur error realted with calloc or realloc function";
            break;
        case ASM_ERROR_GEN_LABLE_TABLE:
            return "The compiler could not generate the label table";
            break;
        case ASM_ERROR_GEN_MACHINE_CODE:
            return "We could not generate the machine code";
            break;
        case ASM_ERROR_CANT_WRITE_INTO_FILE:
            return "There is error with access to file";
            break;
        case ASM_ERROR_INVALID_SYNTAX:
            return "Syntax error";
            break;
        case ASM_ERROR_INVALID_MACHINE_CODE:
            return "Generate invalid machine code";
            break;
        case ASM_ERROR_INVALID_OPERANDS_NUMBER:
            return "Invalid number of operands";
            break;
        case ASM_ERROR_INVALID_OPERAND_SYNTAX:
            return "Invalid operand syntax";
            break;
        case ASM_ERROR_INVALID_OPERAND_TYPE_FOR_COMMAND:
            return "Invalid type of operands";
            break;
        case ASM_ERROR_CANT_READ_LEXEMA:
            return "Error occur, when we try to read lexema from code.\n"
                   "Perhabs this error related with null pointer.";
            break;
        default:
            return "Undefined error code";
            break;
    }
}


struct
{
    /*
    \brief  Функция возвращает имя команды, основываясь на машинном коде
    \param  [in]  cmd  Структура команды
    \return Строка, содержащее имя команды, взятое из таблицы с лексемами
    \note   В случае ошибки будет возвращаться NULL
    */
    static C_string getCommandName(Command cmd)
    {
        cmd.machineCode = getPureMachCode(cmd.machineCode);
        for (int i = 0; i < COMMAND_TABLE_SIZE; i++)
            if (Table[i].machineCode == cmd.machineCode)
                return Table[i].command;
        return NULL;
    }

    /*
    \brief  Функция возвращает имя регистра, основываясь на машинном коде
    \param  [in]  machineCode  Машинный код
    \return Строка, содержащее имя регистра, взятое из таблицы с лексемами
    \note   В случае ошибки будет возвращаться NULL
    */
    static C_string getRegisterName(ui8 machineCode)
    {
        for (int i = 0; i < REGISTER_TABLE_SIZE; i++)
            if (Registers[i].machineCode == machineCode)
                return (C_string)Registers[i].command;
        return NULL;
    }

    /*
    \brief  Функция переводит число шеснадцатиричное представление, возвращая строку с преобразованием
    \param  [in]  num   Число
    \return Строка с шеснадцатиричным числом
    */
    static C_string getStrByNumber(ui32 num)
    {
        static char buff[32] = {};
        sprintf(buff, "0x%X", num);
        return buff;
    }

    /*
    \brief  Функция дизасемблирования команды, результат закидывается в outStream
    \param  [in]  cmd        Команда, которую надо дизасемблировать
    \param  [in]  outStream  Поток вывода, куда будем записывать результат
    */
    static void disasmCommand(Command cmd, FILE* outStream = stdout)
    {
        C_string commandName = NULL;
        C_string operandStr[2] = {NULL, NULL};
        Assert_c(outStream);
        if (!outStream)
            return;
        if (ferror(outStream))
        {
            logger("Disassembler error", "In function %s. Stream \'outStream\' has errors.\n", __FUNCSIG__ );
            return;
        }
            

        commandName = getCommandName(cmd);
        if (!commandName)
        {
            logger("Disassembler error", "The 0x%X code doesn't match any commands", cmd.machineCode & 0xFFFF);
            return;
        }

        bool isInvalidOperands = 0;
        for (int i = 0; i < cmd.nOperands; i++)
        {
            OperandType opType = getOperandType(cmd.machineCode, i);
            if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
                operandStr[i] = getStrByNumber(cmd.operand[i]);
            if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
                operandStr[i] = getRegisterName(cmd.operand[i]);
            isInvalidOperands |= !operandStr[i];
        }

        if (isInvalidOperands)
        {
            logger("Disassembler error", "Invalid code for operands!");
            return;
        }

        fprintf(outStream, "%s ", commandName);
        for (int i = 0; i < cmd.nOperands; i++)
        {
            OperandType opType = getOperandType(cmd.machineCode, i);
            if (opType == OPERAND_REGISTER || opType == OPERAND_NUMBER)
                fprintf(outStream, "%s", operandStr[i]);
            if (opType == OPERAND_MEMORY || opType == OPERAND_MEM_BY_REG)
                fprintf(outStream, "[%s]", operandStr[i]);
            if (cmd.nOperands == 2 && i == 0)
                fprintf(outStream, ", ");
        }
        fprintf(outStream, "\n");
    }


    /*
    \brief  Функция генерации ассемблерного кода, результат закидывается в outStream
    \param  [in]  bytes   Указатель на массив байтов с программой
    \param  [in]  nBytes  Количество байтов в массиве с прогой
    \param  [in]  outStream  Поток вывода, куда будем записывать результат
    \return Возвращает код ошибки или ASM_OK
    */
    static AsmError getCode(ui8* bytes, ui32 nBytes, FILE* outStream = stdout)
    {
        Assert_c(bytes);
        Assert_c(outStream);
        if (!bytes || !outStream)
            return ASM_ERROR_INVALID_INPUT_DATA;
        if (ferror(outStream))
            return ASM_ERROR_CANT_WRITE_INTO_FILE;

        ui8* endPtr = bytes + nBytes;
        ui8* strPtr = bytes;
        Command cmd;
        C_string commandName = NULL;
        C_string operandStr[2] = { NULL, NULL };

        fprintf(outStream, "Offset: Lexema:\n");
        while (bytes < endPtr)
        {
            cmd.machineCode = *((Mcode*)bytes);
            fprintf(outStream, "0x%04X: ", bytes - strPtr);
            bytes += sizeof(Mcode);
            commandName = getCommandName(cmd);
            if (!commandName)
            {
                logger("Disassembler error", "The 0x%X code doesn't match any commands", cmd.machineCode & 0xFFFF);
                return ASM_ERROR_INVALID_MACHINE_CODE;
            }


            cmd.nOperands = getNumberOperands(cmd.machineCode);
            if (cmd.nOperands > 2)
            {
                logger("Disassembler error", "Invalid number of operands: %d, should be less than 3", cmd.nOperands);
                return ASM_ERROR_INVALID_OPERANDS_NUMBER;
            }

            bool isInvalidOperands = 0;
            for (int i = 0; i < cmd.nOperands; i++)
            {
                OperandType opType = getOperandType(cmd.machineCode, i);
                if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
                {
                    cmd.operand[i] = *((ui32*)bytes);
                    bytes += sizeof(ui32);
                    operandStr[i] = getStrByNumber(cmd.operand[i]);

                }
                if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
                {
                    cmd.operand[i] = *((ui8*)bytes);
                    bytes += sizeof(ui8);
                    operandStr[i] = getRegisterName(cmd.operand[i]);
                }
                isInvalidOperands |= !operandStr[i];
            }

            if (isInvalidOperands)
            {
                logger("Disassembler error", "Invalid code for operands!");
                return ASM_ERROR_INVALID_OPERAND_SYNTAX;
            }


            fprintf(outStream, "%s ", commandName);
            for (int i = 0; i < cmd.nOperands; i++)
            {
                OperandType opType = getOperandType(cmd.machineCode, i);
                if (opType == OPERAND_REGISTER || opType == OPERAND_NUMBER)
                    fprintf(outStream, "%s", operandStr[i]);
                if (opType == OPERAND_MEMORY || opType == OPERAND_MEM_BY_REG)
                    fprintf(outStream, "[%s]", operandStr[i]);
                if (cmd.nOperands == 2 && i == 0)
                    fprintf(outStream, ", ");
            }
            fprintf(outStream, "\n");
        }
        return ASM_OK;
    }

}Disassembler;

struct
{
    /*
    \brief Описываем типы лексем
    */
    enum LexemaType
    {
        LEX_ERROR = -1,
        LEX_COMMAND = 1,
        LEX_REGISTER = 2,
        LEX_NUMBER = 3,
        LEX_LABEL = 4,
        LEX_MEMORY = 5,
        LEX_MEM_BY_REG = 6
    };

    /*
    \brief Структура, для работы с метками
    */
    struct Label
    {
        char name[32];
        ui32 pos;
    };

    /*
    \brief Функция проверяет, является ли строка числом
    \param [in]  str  Входящая строка
    \return true, если строка содержит шестнадцатиричное число или просто число,
            в противном случае возвращается false.
    */
    static inline bool isNumber(const char* str)
    {
        Assert_c(str);
        if (!str)
            return false;

        if (strstr(str, "0x"))
            return true;

        if((strstr(str, ".")))
            return true;

        while (*str)
        {
            if (!isdigit(*str))
                return false;
            str++;
        }
        return true;
    }

    /*
    \brief  Функция по строке определяет тип лексемы
    \param  [in]  str  Входящая строка
    \return Тип лексемы, записанной в строке
    */
    static LexemaType getLexemaType(const char* str)
    {
        static char buffer[32] = {};
        Assert_c(str);
        if (!str)
            return LEX_ERROR;

        if (strchr(str, ':'))
            return LEX_LABEL;

        if (isNumber(str))
            return LEX_NUMBER;

        for (int i = 0; i < COMMAND_TABLE_SIZE; i++)
            if (!strcmp(str, Table[i].command))
                return LEX_COMMAND;

        LexemaType result = LEX_ERROR;
        if (strchr(str, '[') && strchr(str, ']'))
        {
            sscanf(str, "[%s]", buffer);
            strchr(buffer, ']')[0] = 0;
            str = buffer;
            result = LEX_MEMORY;
        }

        for (int i = 0; i < REGISTER_TABLE_SIZE; i++)
            if (!strcmp(str, Registers[i].command))
            {
                result = result == LEX_ERROR ? LEX_REGISTER : LEX_MEM_BY_REG;
                break;
            }


        return result;
    }

    /*
    \brief  Функция генерит таблицу меток
    \param  [in]  codeLine  Код программы на ассемблере
    \param  [in]  lables    Указатель на таблицу с метками
    \return Возвращает предполагаемый размер скомпилированного файла в
            байтах. В случае ошибки возвращается константа ASM_ERROR_CODE
    */
    static int genLableTable(const char* codeLine,Label* lables)
    {
        Assert_c(codeLine);
        Assert_c(lables);
        if (!codeLine || !lables)
            return ASM_ERROR_CODE;

        static char buf[32] = {};
        ui32 nBytes = 0;
        LexemaType lxt;
        while (*codeLine)
        {
            sscanf(codeLine, "%s ", buf);
            codeLine += strlen(buf) + 1;
            lxt = getLexemaType(buf);
            switch (lxt)
            {
                case LEX_COMMAND:
                    nBytes+=sizeof(Mcode);
                    break;
                case LEX_LABEL:
                    lables->pos = nBytes;
                    memcpy(lables->name, buf, 32);
                    *(strchr(lables->name, ':')) = 0;
                    lables++;
                    break;
                case LEX_NUMBER:
                    nBytes += sizeof(ui32);
                    break;
                case LEX_REGISTER:
                    nBytes++;
                    break;
                case LEX_MEMORY:
                    nBytes += sizeof(ui32);
                    break;
                case LEX_MEM_BY_REG:
                    nBytes += sizeof(ui8);
                    break;
                default:
                    nBytes += sizeof(ui32);
                    break;
            }
        }
        return nBytes;
    }

    /*
    \brief  Функция выполняет поиск метки в таблице по имени
    \param  [in]  labelName  Имя метки
    \param  [in]  lables     Таблица с метками
    \param  [in]  nLables    Количество меток в таблице
    \return Индекс метки в таблице, в случае ошибки возвращается ASM_ERROR_CODE
    */
    static int getLabel(char* labelName, Label* lables, ui32 nLables)
    {
        Assert_c(labelName);
        Assert_c(lables);
        if (!labelName || !lables)
            return ASM_ERROR_CODE;

        for (int i = 0; i < nLables; i++)
            if (!strcmp(lables[i].name, labelName))
                return i;
        return ASM_ERROR_CODE;
    }


    /*
    \brief   Функция парсит лексему из строки
    \details Функция вытаскивает лексему из строки, переводя указатель на новую лексему.
             Затем происходит опеределение полученной лексемы.
             В зависимости от типа будет возвращает указатель на буффер, содержащий подробную
             информацию о лексеме.
    \param  [in,out]  ptrCodeLine  Указатель на строку с кодом
    \param  [in,out]  lxt          Указатель на структуру LexemaType
    \return Указатель на буфер, с подробной информацией о лексеме. В случае ошибки возвращается NULL
            Например, если лексема является числом, то возвращается указатель на ui32, в котором лежит именно это число.
            Если лексема была командой или регистром, то возвращается указатель на ui32, в котором лежит машинный код.
    \note   Важно отметить, что после выполнения данной функции значение указателя (*ptrCodeLine) изменяется!
    */
    static inline void* readNextLexema(char** ptrCodeLine,LexemaType* lxt)
    {
        Assert_c(ptrCodeLine);
        Assert_c(lxt);
        if (!ptrCodeLine || !lxt)
            return NULL;
        Assert_c(*ptrCodeLine);
        if(!*ptrCodeLine)
            return NULL;

        static char buffer[32] = {};
        static ui32 bufInt = 0;
        sscanf(*ptrCodeLine, "%s ", buffer);
        *ptrCodeLine = strchr(*ptrCodeLine, ' ') + 1;

        *lxt = getLexemaType(buffer);
        
        if(*lxt == LEX_COMMAND)
            for(int i=0;i<COMMAND_TABLE_SIZE;i++)
                if (!strcmp(buffer, Table[i].command))
                {
                    bufInt = Table[i].machineCode;
                    return &bufInt;
                }
        if (*lxt == LEX_REGISTER)
            for (int i = 0; i<REGISTER_TABLE_SIZE; i++)
                if (!strcmp(buffer, Registers[i].command))
                {
                    bufInt = Registers[i].machineCode;
                    return &bufInt;
                }
        if (*lxt == LEX_NUMBER)
        {
            if (strchr(buffer, 'x'))
            {
                sscanf(buffer, "0x%X", &bufInt);
                return &bufInt;
            }
            if (strchr(buffer, '.'))
            {
                sscanf(buffer, "%f", &bufInt);
                return &bufInt;
            }
            else
            {
                sscanf(buffer, "%d", &bufInt);
                return &bufInt;
            }
        }

        return &buffer;
    }

    /*
    \brief  Функция парсит операнд, если его лексема имела тип LEX_MEMORY или LEX_MEM_BY_REG
    \param  [in]  str       Указатель на строку с лексемой
    \param  [in]  lables    Указатель на таблицу с метками
    \param  [in]  nLables   Количество меток в таблице
    \return В зависимости от типа лексемы будет возвращается следующие величины:
            Если тип лексемы LEX_MEMORY, то возвращается число, равное семещению регистра eip
            Если тип лексемы LEX_MEM_BY_REG, то возвращается код регистра
            В случае ошибки возвращается константа ASM_ERROR_CODE.
    */
    static int getMemoryOperand(char* str, Label* lables, ui32 nLables)
    {
        Assert_c(str);
        if (!str)
            return ASM_ERROR_CODE;
        if (str[0] != '[')
            return ASM_ERROR_CODE;
        str++;

        Assert_c(lables);
        if (!lables)
            return ASM_ERROR_CODE;

        char* endStr = strchr(str, ']');
        Assert_c(endStr);
        if (!endStr)
            return ASM_ERROR_CODE;
        *endStr = 0;
        
        int resOperand = 0;

        if (isNumber(str))
        {
            if (strchr(str, 'x'))
                sscanf(str, "0x%X", &resOperand);
            else
                sscanf(str, "%d", &resOperand);
            return resOperand;
        }

        resOperand = getLabel(str, lables, nLables);
        if (resOperand != ASM_ERROR_CODE)
            return lables[resOperand].pos;

        for (int i = 0; i < REGISTER_TABLE_SIZE; i++)
            if (!strcmp(str, Registers[i].command))
                return Registers[i].machineCode;

        return ASM_ERROR_CODE;
    }


    /*
    \brief  Функция проверяет, что сгенерированный машинный код, допускает указанные типы операндов
    \param  [in]  cmd  Структура с командой
    \return Код ошибки. Если команда сгенерирована правильно, то 
            возвращается ASM_OK
    */
    static AsmError checkValidityOfOperands(Command cmd)
    {
        char* validityStr[2] = {NULL, NULL};
        Mcode pureMachineCode = getPureMachCode(cmd.machineCode);
        for (int i = 0; i < COMMAND_TABLE_SIZE; i++)
            if (Table[i].machineCode == pureMachineCode)
            {
                validityStr[0] = Table[i].validFirstOperand;
                validityStr[1] = Table[i].validSecondOperand;
            }

        if (validityStr[0] == NULL)
            return ASM_ERROR_INVALID_MACHINE_CODE;

        const char* opTypeStr = "RNMB";
        OperandType opType;
        for (int i = 0; i < cmd.nOperands; i++)
        {
            opType = getOperandType(cmd.machineCode, i);
            if (!strchr(validityStr[i], opTypeStr[(ui8)opType]))
                return ASM_ERROR_INVALID_OPERAND_TYPE_FOR_COMMAND;
        }
        return ASM_OK;
    }

    /*
    \brief  Функция генерит машинный код
    \param  [in]     codeLine  Строка
    \param  [in]     lables    Таблица с метками
    \param  [in]     nLables   Количество меток в таблице
    \param  [in,out] ptrBytes  Указатель на массив байтов, куда будем записывать результат
    \return Код ошибки. В случае успеха возвращается ASM_OK.
    */
    static AsmError genBytes(const char* codeLine,Label* lables, ui32 nLables,ui8* ptrBytes)
    {
        Assert_c(codeLine);
        Assert_c(lables);
        Assert_c(ptrBytes);
        if (!codeLine || !lables || !ptrBytes)
            return ASM_ERROR_INVALID_INPUT_DATA;


        char* ptr = NULL;
        ui8* bytes = ptrBytes;
        int currentPosition = 0;
        LexemaType lxt;
        Command cmd;
        while (*codeLine)
        {
            ptr = (char*)readNextLexema((char**)&codeLine,&lxt);
            if (lxt == LEX_LABEL)
                continue;
            if (lxt != LEX_COMMAND)
            {
                logger("Compilator error", "Lexema: \"%s\" is not command, but should be...",ptr);
                return ASM_ERROR_INVALID_SYNTAX;
            }
            if (!ptr)
            {
                logger("Compilator error", "We can't read lexema.\n");
                return ASM_ERROR_CANT_READ_LEXEMA;
            }
            
            cmd.machineCode = *((Mcode*)ptr);
            currentPosition += sizeof(Mcode);
            cmd.nOperands = getNumberOperands(cmd.machineCode);
            for (int i = 0; i < cmd.nOperands; i++)
            {
                ptr = (char*)readNextLexema((char**)&codeLine, &lxt);
                if (!ptr)
                {
                    logger("Compilator error", "We can't read lexema.\n");
                    return ASM_ERROR_CANT_READ_LEXEMA;
                }
                if (lxt == LEX_NUMBER)
                {
                    cmd.operand[i] = *((ui32*)ptr);
                    setOperandType(&cmd.machineCode, i, OPERAND_NUMBER);
                    currentPosition += sizeof(ui32);
                }
                if (lxt == LEX_REGISTER)
                {
                    cmd.operand[i] = *((ui8*)ptr);
                    setOperandType(&cmd.machineCode, i, OPERAND_REGISTER);
                    currentPosition += sizeof(ui8);
                }
                if (lxt == LEX_MEMORY || lxt == LEX_MEM_BY_REG)
                {
                    cmd.operand[i] = getMemoryOperand(ptr,lables,nLables);
                    if (cmd.operand[i] == ASM_ERROR_CODE)
                    {
                        logger("Compilator error", "Invalid link to memory: \"%s\" ", ptr);
                        return ASM_ERROR_INVALID_OPERAND_SYNTAX;
                    }
                    setOperandType(&cmd.machineCode, i, lxt == LEX_MEMORY ? OPERAND_MEMORY : OPERAND_MEM_BY_REG);
                    currentPosition += lxt == LEX_MEMORY ? sizeof(ui32) : sizeof(ui8);
                }
                if (lxt == LEX_LABEL)
                {
                    logger("Compilator error", "Invalid operand: \"%s\" ", ptr);
                    return ASM_ERROR_INVALID_OPERAND_SYNTAX;
                }
                if (lxt == LEX_ERROR)
                {
                    int labelIndex = getLabel(ptr, lables, nLables);
                    if (labelIndex == -1)
                    {
                        logger("Compilator error", "Invalid lexema: \"%s\" ", ptr);
                        return ASM_ERROR_INVALID_SYNTAX;
                    }
                    setOperandType(&cmd.machineCode, i, OPERAND_NUMBER);
                    currentPosition += sizeof(ui32);
                    cmd.operand[i] = lables[labelIndex].pos;
                }
                
            }
            
            *((Mcode*)bytes) = cmd.machineCode;
            bytes += sizeof(Mcode);


            AsmError errorCode = checkValidityOfOperands(cmd);
            if (errorCode == ASM_ERROR_INVALID_OPERAND_TYPE_FOR_COMMAND)
            {
                logger("Compilator error", "Invalid type of operand for current command.");
                Disassembler.disasmCommand(cmd, getLoggerStream());
                return ASM_ERROR_INVALID_OPERAND_TYPE_FOR_COMMAND;
            }
            if (errorCode == ASM_ERROR_INVALID_MACHINE_CODE)
            {
                logger("Compilator error", "Has been generated invalid machine code: 0x%X",cmd.machineCode);
                return ASM_ERROR_INVALID_MACHINE_CODE;
            }


            for (int i = 0; i < cmd.nOperands; i++)
            {
                OperandType opType = getOperandType(cmd.machineCode, i);
                if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
                {
                    *((ui32*)bytes) = cmd.operand[i];
                    bytes += sizeof(ui32);
                }
                if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
                {
                    *((ui8*)bytes) = cmd.operand[i];
                    bytes += sizeof(ui8);
                }
            }
        }
        return ASM_OK;
    }

    /*
    \brief  Функция записывает сгенерированный машинный код в поток вывода
    \param  [in]  bytes      Указатель на массив байтов
    \param  [in]  nBytes     Количество байтов в массиве
    \param  [in]  outStream  Поток вывода
    */
    static AsmError throwMachineCodeIntoStream(ui8* bytes, ui32 nBytes, FILE* outStream = stdout)
    {
        Assert_c(bytes);
        Assert_c(outStream);
        if (!bytes || !outStream)
            return ASM_ERROR_INVALID_INPUT_DATA;
        if (ferror(outStream))
        {
            logger("Access error", "In function %s. There are problems with file stream.", __FUNCSIG__ );
            return ASM_ERROR_CANT_WRITE_INTO_FILE;
        }

        if (outStream == stdout)
        {
            for (int i = 0; i < nBytes; i++)
            {
                printf("0x%02X ", bytes[i] & 0xFF);
                if ( (i+1) % 16 == 0)
                    printf("\n");
            }
        }
        else
            fwrite(bytes, sizeof(ui8), nBytes, outStream);

        return ASM_OK;
    }

    /*
    \brief  Функция инициализирует таблицу меток, затем генерит машинный код;
            Результат закидывается в outStream
    \param  [in]  codeLine  Строка с кодом на ассемблере
    \param  [in]  outStream  Поток вывода
    \return Код ошибки. В случае успеха возвращается ASM_OK.
    */
    static AsmError getCode(const char* code, FILE* outStream = stdout)
    {
        Assert_c(code);
        Assert_c(outStream);
        if (!code || !outStream)
            return ASM_ERROR_INVALID_INPUT_DATA;
        if (ferror(outStream))
            return ASM_ERROR_CANT_WRITE_INTO_FILE;


        //Из строки с исходным кодом программы удаляем все лишние символы,
        //оставляя только лексемы языка. В качестве разделителя используем пробел.
        C_string codeLine = NULL;
        ui32 nLabels = 0;
        {
            int strLen = strlen(code);
            codeLine = (C_string)calloc(strLen + 2, sizeof(ui8));
            Assert_c(codeLine);
            if (!codeLine)
                return ASM_ERROR_OUT_OF_MEMORY;
            ui8 c[2] = {};
            bool pingpong = 0;
            int index = 0;
            bool isComment = 0;
            for (int i = 0; i < strLen; i++)
            {
                c[pingpong] = code[i];
                if (c[pingpong] == ':')
                    nLabels++;

                if (c[pingpong] == ';')
                {
                    while(i < strLen && code[i] != '\n')
                        i++;
                    if (i == strLen)
                        break;
                    c[pingpong] = code[i];
                }
                
                #define isValidChr(x) (isalpha(x) || isdigit(x) || strchr(":_[].-;" , x) && x )

                if (!isValidChr(c[pingpong]) && !isValidChr(c[pingpong ^ 1]))
                    continue;
                else if (isValidChr(code[i]))
                    codeLine[index] = code[i];
                else
                    codeLine[index] = ' ';
                index++;
                #undef isValidChr(x)

                pingpong ^= 1;
            }
            if (codeLine[index - 1] != ' ')
                codeLine[index++] = ' ';
            codeLine[index++] = 0;
            codeLine = (C_string)realloc(codeLine, index * sizeof(ui8));
            Assert_c(codeLine);
            if (!codeLine)
                return ASM_ERROR_OUT_OF_MEMORY;
        }



        // Генерируем таблицу меток
        Label* lables = (Label*)calloc(nLabels, sizeof(Label));
        Assert_c(lables);
        if (!lables)
        {
            free(codeLine);
            return ASM_ERROR_OUT_OF_MEMORY;
        }
        ui32 nBytes = genLableTable(codeLine, lables);
        Assert_c(nBytes != ASM_ERROR_CODE);
        if (nBytes == ASM_ERROR_CODE)
        {
            free(codeLine);
            free(lables);
            return ASM_ERROR_GEN_LABLE_TABLE;
        }
        //выделяем память для проги
        ui8* bytes = (ui8*)calloc(nBytes + 1, sizeof(ui8));
        Assert_c(bytes);
        if (!bytes)
        {
            free(codeLine);
            free(lables);
            return ASM_ERROR_OUT_OF_MEMORY;
        }

        AsmError errorCode = ASM_OK;

        //производим компиляцию
        errorCode = genBytes(codeLine,lables, nLabels, bytes);
        if (errorCode != ASM_OK)
        {
            free(codeLine);
            free(lables);
            free(bytes);
            return ASM_ERROR_GEN_MACHINE_CODE;
        }

        //и выводим результат компиляции в поток вывода
        errorCode = throwMachineCodeIntoStream(bytes, nBytes, outStream);
        
        free(codeLine);
        free(lables);
        free(bytes);

        if (errorCode != ASM_OK)
            return ASM_ERROR_CANT_WRITE_INTO_FILE;

        return ASM_OK;
    }

}Compiler;

/*
\brief  Функция, компилирующая ассемблерный код
\param  [in]  code       Строка с исходным текстом программы
\param  [in]  outStream  Поток вывода (может быть файлом)
\return Код ошибки. В случае успеха возвращается ASM_OK.
*/
AsmError compile(const char* code, FILE* outStream)
{
    return Compiler.getCode(code, outStream);
}

/*
\brief  Производящая дизасемблирование бинарника
\param  [in]  code       Строка байтов, которую будем дизасемблировать
\param  [in]  size       Количество байтов в массиве
\param  [in]  outStream  Поток вывода (может быть файлом)
\return Код ошибки. В случае успеха возвращается ASM_OK.
*/
AsmError disasm(const ui8* code, int size, FILE* outStream)
{
    Assert_c(size > 0);
    if (size <= 0)
        return ASM_ERROR_INVALID_INPUT_DATA;
    return Disassembler.getCode((ui8*)code, size, outStream);
}


/*
\brief  Функция дизасемблирования команды, результат закидывается в outStream
\param  [in]  cmd        Команда, которую надо дизасемблировать
\param  [in]  outStream  Поток вывода, куда будем записывать результат
*/
void disasmCommand(Command cmd, FILE* outStream)
{
    Disassembler.disasmCommand(cmd, outStream);
}
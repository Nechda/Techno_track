#include "Stack_kernel.h"
#include <string.h>

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
static Hash getHash(const ui8* data, ui32 len)
{
    Hash hash = 0;
    ui8 h = 0;
    ui8* hPtr = (ui8*)&hash;
    for (int j = 0; j < 8; j++)
    {
        h = T[(data[0] + j) % 256];
        for (int i = 0; i < len; i++)
            h = T[h^data[i]];
        hPtr[j] = h;
    }
    return hash;
}

/**
\brief   Функция изменения размера стека
\param   [in]   stk       Указатель на структура стека
\param   [in]   newSize   Новый размер выделенной памяти
\return  Возвращается 0 если все ok, в противном случае код ошибки.
*/
static int stackResize(void* stk,ui32 newSize = 1)
{
    Assert_c(stk);
    if (!stk)
        return STK_ERROR_NULL_PTR;

    _BaseStack* stack = (_BaseStack*)stk;
    char* ptr = (char*)stack->data;
    if (ptr == NULL)
    {
        ptr = (char*)calloc(newSize*stack->elementSize + 2 * sizeof(CanaryType),sizeof(ui8));
        Assert_c(ptr);
        if (!ptr)
            return STK_ERROR_OUT_OF_MEMORY;
    }
    else
    {
        ptr -= sizeof(CanaryType);
        ptr = (char*)realloc(ptr, newSize*stack->elementSize + 2 * sizeof(CanaryType));
        Assert_c(ptr);
        if (!ptr)
            return STK_ERROR_OUT_OF_MEMORY;
    }

    stack->data = ptr + sizeof(CanaryType);

    *((CanaryType*)ptr) = STK_CANARY_VALUE;
    ptr += sizeof(CanaryType) + newSize*stack->elementSize;
    *((CanaryType*)ptr) = STK_CANARY_VALUE;

    stack->capacity = newSize;
    return 0;
}

/**
\brief   Функция вычисления хешей стека
\param   [in]   stk   Указатель на стуктуру стека
*/
static inline void recalcHashes(void* stk)
{
    Assert_c(stk);
    if (!stk)
        return;

    _BaseStack* stack = (_BaseStack*)stk;
    stack->dataHash = getHash((ui8*)stack->data - sizeof(CanaryType), stack->capacity*stack->elementSize + sizeof(CanaryType));
    stack->structHash = getHash((ui8*)&stack->dataHash, (ui8*)&stack->rightSide - (ui8*)&stack->dataHash + sizeof(CanaryType));
}

/**
\brief   Функция, инициализирующая стек начальными данными
\param   [in]   stk            Указатель на структуру стека
\param   [in]   capacity       Емкость нового стека
\param   [in]   elementsSize   Размер одного элемента стека(в байтах)
\return  Возвращается 0 если все ok, в противном случае код ошибки.
*/
int stackInit_(void* stk,const ui32 capacity,const ui32 elementSize)
{
    Assert_c(stk);
    if (!stk)
        return STK_ERROR_NULL_PTR;
    int errorCode = 0;

    _BaseStack* stack = (_BaseStack*)stk;
    stack->elementSize = elementSize;
    stack->capacity = capacity;
    stack->size = 0;
    stack->data = NULL;
    errorCode = stackResize(stack, capacity);
    Assert_c(!errorCode);
    if (errorCode)
        return errorCode;
    recalcHashes(stack);
}


/**
\brief   Валидатор стека
\param   [in]   stk   Указатель на структуру стека
\return  Возвращается 0 если все ok, в противном случае код ошибки.
*/
int stackValidity(const void* stk)
{
    Assert_c(stk);
    if (!stk)
        return STK_ERROR_NULL_PTR;

    _BaseStack* stack = (_BaseStack*)stk;
    Assert_c(stack->data);
    if (!stack->data)
        return STK_ERROR_INVALID_PTR;

    if (stack->capacity < stack->size)
    {
        Assert_c(!"Capacity less than size!\n");
        return STK_ERROR_OUT_OF_RANGE;
    }

    if (stack->leftSide != STK_CANARY_VALUE)
    {
        Assert_c(!"Somebody attack us from left side!\n");
        return STK_ERROR_ATTACK;
    }
    if (stack->rightSide != STK_CANARY_VALUE)
    {
        Assert_c(!"Somebody attack us from right side!\n");
        return STK_ERROR_ATTACK;
    }

    
    Hash structHash = getHash((ui8*)&stack->dataHash, (ui8*)&stack->rightSide - (ui8*)&stack->dataHash + sizeof(CanaryType));
    if (structHash != stack->structHash)
    {
        Assert_c(!"Somebody has changed our stack structure!\n");
        return STK_ERROR_CHANGED_STRUCTURE;
    }

    Hash dataHash = getHash((ui8*)stack->data - sizeof(CanaryType), stack->capacity*stack->elementSize + sizeof(CanaryType));
    if (dataHash != stack->dataHash)
    {
        Assert_c(!"Somebody has changed our data in stack!\n");
        return STK_ERROR_CHANGED_DATA;
    }

    CanaryType* ptr = (CanaryType*)stack->data;
    ptr--;
    if (*ptr != STK_CANARY_VALUE)
    {
        Assert_c(!"Somebody attack us from left side!\n");
        return STK_ERROR_ATTACK;
    }

    ptr = (CanaryType*)((ui8*)stack->data + stack->capacity*stack->elementSize);
    if ( *ptr != STK_CANARY_VALUE)
    {
        Assert_c(!"Somebody attack us from right side!\n");
        return STK_ERROR_ATTACK;
    }

    ptr = (CanaryType*)stack->data;
    ptr--;

    return 0;
}

/**
\brief   Функция, записывающая на вершину стека данные
\param   [in]   stk   Указатель на структуру стека
\param   [in]   value Указатель на данные, которые будем записывать
\return  Возвращается 0 если все ok, в противном случае код ошибки.
*/

int stackPush_(void* stk,void* value)
{
    int errorCode = 0;
    errorCode = stackValidity(stk);
    if (errorCode)
        return errorCode;

    _BaseStack* stack = (_BaseStack*)stk;
    if (stack->size == stack->capacity)
        stackResize(stack,stack->capacity+STK_BUFFER_ADDITION);

    char* addr = (char*)stack->data;
    addr += stack->elementSize * stack->size;
    memcpy(addr, value, stack->elementSize);
    stack->size++;

    recalcHashes(stk);
    errorCode = stackValidity(stk);
    if (errorCode)
        return errorCode;

    return 0;
}


/**
\brief   Функция, вытаскивающая из стека данные
\param   [in]   stk   Указатель на структуру стека
\param   [in]   dest  Указатель на память, куда будет выгружаться элемент
\return  Возвращается 0 если все ok, в противном случае код ошибки.
\note    Если в стеке нет элементов, то функция возвращает код ошибки STK_ERROR_STK_IS_EMPTY
*/
int stackPop_(void* stk,void* dest)
{
    int errorCode = 0;
    errorCode = stackValidity(stk);
    if (errorCode)
        return errorCode;

    Assert_c(dest);
    if (!dest)
        return STK_ERROR_INVALID_PTR;

    _BaseStack* stack = (_BaseStack*)stk;
    if (!stack->size)
        return STK_ERROR_STK_IS_EMPTY;

    stack->size--;
    char* addr = (char*)stack->data;
    addr += stack->elementSize * stack->size;
    memcpy(dest, addr, stack->elementSize);
    memset(addr, 0, stack->elementSize);

    recalcHashes(stk);
    errorCode = stackValidity(stk);
    if (errorCode)
        return errorCode;

    return 0;
}


/**
\brief   Функция, уничтожающая стек
\param   [in]   stk   Указатель на структуру стека
*/
void stackDest_(void* stk)
{
    Assert_c(stk);
    if (!stk)
        return;
    
    _BaseStack* stack = (_BaseStack*)stk;
    ui8* ptr = (ui8*)stack->data;
    ptr -= sizeof(CanaryType);
    memset(ptr, 0, 2 * sizeof(CanaryType) + stack->capacity*stack->elementSize);
    free(ptr);
}

/**
\brief   Функция дампа, полностью печатает иформацию о стеке.
\detail  Функция дампа, выводит полную информацию о стеке, включая адреса полей стека и их значения.
         Также вывовдит информацию откуда данный дамп был вызван и какое именно имя переменной
         было переданно в функцию dump(...).
\param   [in]   stk            Указатель на структуру стека
\param   [in]   file           Строка, содержащая имя файла, из которого был вызван дамп (в макросе подставляется __FILE__)
\param   [in]   func           Строка, содержащая имя функции, из которой был вызван дамп (в макросе подставляется __FUNCSIG__ vs2017)
\param   [in]   line           Номер строки, из которой был вызван дамп (в макросе подставляется __LINE__)
\param   [in]   variableName   Имя переменной, которая передается в дамп
*/
void stackDump_(void* stk, const char* file, const char* func, const ui32 line,const char* variableName)
{
    if (!stk)
    {
        Assert_c(!"stk pointer contain NULL!\n");
        return;
    }

    _BaseStack* stack = (_BaseStack*)stk;
    ui32 elementSize = stack->elementSize;
    ui32 capacity = stack->capacity;
    ui32 size = stack->size;
    char* ptrData = (char*)stack->data;
    printf("Dump has called from\nfile:%s\nfunction:%s\nline: %d\n", file, func, line);
    printf("Stack[0x%X] (variable name: %s) {\n", (char*)stk,variableName);
    printf("   canary{\n");
    printf("       leftSide [0x%X] = 0x%llX\n", &stack->leftSide, stack->leftSide);
    printf("       rightSide[0x%X] = 0x%llX\n", &stack->rightSide, stack->rightSide);
    printf("   }\n");
    printf("   structHash   [0x%X] = 0x%llX\n", &stack->structHash, stack->structHash);
    printf("   dataHash     [0x%X] = 0x%llX\n", &stack->dataHash, stack->dataHash);
    printf("   elementSize  [0x%X] = %d\n", &stack->elementSize, stack->elementSize);
    printf("   size         [0x%X] = %d\n", &stack->size, stack->size);
    printf("   capacity     [0x%X] = %d\n", &stack->capacity, stack->capacity);
    printf("   data         [0x%X] = 0x%X\n", &stack->data, stack->data);
    printf("   {\n");
    printf("       canary{\n");
    printf("          leftSide[0x%X] = 0x%llX\n", ptrData - sizeof(CanaryType),   *((CanaryType*)(ptrData - sizeof(CanaryType))) );
    printf("          leftSide[0x%X] = 0x%llX\n", ptrData + capacity*elementSize, *((CanaryType*)(ptrData + capacity*elementSize)));
    printf("       }\n");
    for (int i = 0; i < capacity; i++)
    {
        printf("        ");
        if (i < size)
            printf("*");
        else
            printf(" ");
        printf("data[%d] = 0x",i);
        for (int j = 0; j < elementSize; j++)
            printf("%X", *((char*)stack->data + i*elementSize + j) & 0xFF);
        printf("\n");
    }
    printf("   }\n");
    printf("}\n");

}
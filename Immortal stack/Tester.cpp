#include "Tester.h"
#include <ctime>
#include <stdio.h>
#include <string.h>


#define TYPE_ int
#include "Stack.h"
#undef TYPE_


const int STACK_LENGTH = 10; ///< размер стека во всех тестах
static int numberOfTest = 0; ///< номер теста, который в данный момент исполняется

static inline void swap(int* ptr1, int* ptr2) ///< функця, меняющая два числа местами
{
    *ptr1 ^= *ptr2;
    *ptr2 ^= *ptr1;
    *ptr1 ^= *ptr2;
}

static void generateStack(Stack(int)* stack) ///< функция, генерирующая стек, запоняя его случайным образом
{
    stackInit(stack, STACK_LENGTH);
    srand(time(0));

    int* arr = (int*)calloc(STACK_LENGTH, sizeof(int));
    for (int i = 0; i < STACK_LENGTH; i++)
    {
        arr[i] = rand() % 100000;
        stackPush(stack, &arr[i]);
    }
    free(arr);
}

static void shuffleArray(int* arr, int size) ///< функция, перемешивающая массив
{
    srand(time(0));
    for (int i = 0; i < size; i++)
    {
        int i1 = i;
        int i2 = rand() % size;
        arr[i1] ^= arr[i2];
        arr[i2] ^= arr[i1];
        arr[i1] ^= arr[i2];
    }
}

/**
\brief Функция, выводящая подробную информацию о результате теста
\param   [in]   stack                     Указатель на структуру стека
\param   [in]   error                     Ожидаемый код ошибки
\param   [in]   strTestInfo               Иформация о том, что именно проверялось в тесте
\param   [in]   strTestErrorExplanation   Сообщение, которое выводится, в случае, когда тест провален
*/
static inline void stackValidityCheck(Stack(int)* stack, const int error, const char* strTestInfo, const char* strTestErrorExplanation)
{
    numberOfTest++;
    int errorCode = _stackValidity(stack);

    if (errorCode != error && error)
    {
        printf("Test #%d:   %s\n",numberOfTest, strTestInfo);
        printf("_______wrong_______\n");
        printf("%s\n", strTestErrorExplanation);
        stackDump(*stack);
    }

    if (errorCode != error && error)
    {
        printf("Test #%d:   Check for validity...\n",numberOfTest);
        printf("_______wrong_______\n");
        printf("Validity: %d, but should be 0 ", errorCode);
        stackDump(*stack);
    }
    
    if (errorCode == error)
        printf("Test #%d:   Ok!\n", numberOfTest);
}

void test_pushData() ///< Тестируем стандартные функции стека push и pop
{
    Stack(int) stack;
    generateStack(&stack);

    const int N = 10;
    int* arr = (int*)calloc(N, sizeof(int));
    for (int i = 0; i < N; i++)
    {
        arr[i] = rand() % 100000;
        stackPush(&stack, &arr[i]);
    }
    for (int i = N - 1; i >= 0; i--)
    {
        int ans = 0;
        stackPop(&stack, &ans);
        if (ans != arr[i]+1) ///< намеренная ошибка приводит к тому, что дампы выводятся в логи
        {
            printf("Test failed!\n");
            for (int j = 0; j <= i; j++)
                printf("arr[%d]=%d\n", j, arr[j]);
            stackDump(stack);
            printf("pop stack give us: %d, but should %d\n", ans, arr[i]);
        }
    }
    stackDump(stack);
    free(arr);
    stackDest(&stack);
}

#ifndef NDEBUG

void test_changeStructure_editHash() ///< Тестируем валидатор, который должен отловить изменение поля структуры
{
    Stack(int) stack;
    generateStack(&stack);


    int errorCode = 0;

    //попробуем поменять хеш для данных специально
    {
        Hash dataHash = stack.dataHash;
        stack.dataHash = 0;
        stackValidityCheck(
            &stack,
            STK_ERROR_CHANGED_STRUCTURE,
            "Edit dataHash...",
            "dataHash has changed, but validator doesn't catch this error!"
        );
        stack.dataHash = dataHash;
    }
    
    stackValidityCheck(
        &stack,
        0,
        NULL,
        NULL
    );

    _stackPush(&stack, NULL);

    stackDest(&stack);
}

void test_changeStructure_editCapacity() ///< Тестируем валидатор, который должен отловить изменение поля структуры
{
    Stack(int) stack;
    generateStack(&stack);


    int errorCode = 0;

    //теперь поменяем емкость стека
    {
        ui32 cap = stack.capacity;
        stack.capacity++;
        stackValidityCheck(
            &stack,
            STK_ERROR_CHANGED_STRUCTURE,
            "Edit capacity value...",
            "Capacity has changed, but validator doesn't catch this error!"
        );
        stack.capacity = cap;
    }
    stackValidityCheck(
        &stack,
        0,
        NULL,
        NULL
    );

    stackDest(&stack);
}

void test_changeStructure_leftSideAttack() ///< Тестируем валидатор, который должен отловить изменение канареечной переменной
{
    Stack(int)* stack = (Stack(int)*)calloc(1,sizeof(Stack(int)));
    generateStack(stack);


    int errorCode = 0;
    {
        char* ptr = (char*)&stack->leftSide;
        *ptr++;
        stackValidityCheck(
            stack,
            STK_ERROR_ATTACK,
            "Edit canary cell in structure...",
            "Cnary variable has changed, but validator doesn't catch this error!"
        );
        *ptr--;
    }
    stackValidityCheck(
        stack,
        0,
        NULL,
        NULL
    );

    stackDest(stack);
    free(stack);
}

void test_changeStructure_rightSideAttack() ///< Тестируем валидатор, который должен отловить изменение канареечной переменной
{
    Stack(int)* stack = (Stack(int)*)calloc(1, sizeof(Stack(int)));
    generateStack(stack);


    int errorCode = 0;
    {
        char* ptr = (char*)&stack->rightSide;
        *ptr++;
        stackValidityCheck(
            stack,
            STK_ERROR_ATTACK,
            "Edit canary cell in structure...",
            "Cnary variable has changed, but validator doesn't catch this error!"
        );
        *ptr--;
    }
    stackValidityCheck(
        stack,
        0,
        NULL,
        NULL
    );

    stackDest(stack);
    free(stack);
}

void test_changeData_editBits() ///< Тестируем валидатор, который должен отловить изменение битов в массиве данных
{
    Stack(int) stack;
    generateStack(&stack);


    int errorCode = 0;

    //пробуем отследить манипуляцию с данными 
    for (int i = 0; i < STACK_LENGTH; i++)
        for (int j = 0; j < 8; j++)
        {
            int* data = stack.data;
            data[i] ^= 1 << j;
            stackValidityCheck(
                &stack,
                STK_ERROR_CHANGED_DATA,
                "Change one bit in array...",
                "One bit in data has changed, but validator doesn't catch this error!"
            );

            data[i] ^= 1 << j;
        }

    stackValidityCheck(
        &stack,
        0,
        NULL,
        NULL
    );

    stackDest(&stack);
}

void test_changeData_swapElements() ///< Тестируем валидатор, который должен отловить изменение порядка в данных
{
    Stack(int) stack;
    generateStack(&stack);


    int errorCode = 0;
    //поменяем местами 2 значения

    for(int i=0;i<STACK_LENGTH;i++)
        for(int j = i+1; j<STACK_LENGTH;j++)
        {
            int* data = stack.data;
            swap(&data[i], &data[j]);
            errorCode = _stackValidity(&stack);

            stackValidityCheck(
                &stack,
                STK_ERROR_CHANGED_DATA,
                "Swap two elements in array...",
                "Pair of elemets has swapped, but validator doesn't catch this error!"
            );

            swap(&data[i], &data[j]);

        }

    stackValidityCheck(
        &stack,
        0,
        NULL,
        NULL
    );


    stackDest(&stack);
}

void test_changeData_randomEditElemtnts() ///< Тестируем валидатор, который должен отловить измение данных в массиве, количество изменений и сами изменения случайны
{
    Stack(int) stack;
    generateStack(&stack);

    int errorCode = 0;

    int countEditElement = rand() % (STACK_LENGTH-1) + 1;
    int editedData[STACK_LENGTH];
    int indexOfEditedElemet[STACK_LENGTH];

    for (int i = 0; i < STACK_LENGTH; i++)
        indexOfEditedElemet[i] = i;
    shuffleArray(indexOfEditedElemet, STACK_LENGTH);
    for (int i = 0; i < STACK_LENGTH; i++)
        editedData[i] = rand() % (100000/2) - 100000;


    for (int i = 0; i < countEditElement; i++)
        stack.data[i] = editedData[indexOfEditedElemet[i]];

    stackValidityCheck(
        &stack,
        STK_ERROR_CHANGED_DATA,
        "Change random elements to random value in array ...",
        "We changed data, but validator doesn't catch this error!"
    );


    stackDest(&stack);
        
}

void test_changeData_leftSideAttack() ///< Тестируем валидатор, который должен отловить изменение канареечной переменной для массива данных
{
    Stack(int)* stack = (Stack(int)*)calloc(1, sizeof(Stack(int)));
    generateStack(stack);


    int errorCode = 0;
    {
        char* ptr = (char*)&stack->data-sizeof(CanaryType);
        *ptr++;
        stackValidityCheck(
            stack,
            STK_ERROR_ATTACK,
            "Edit canary cell in structure...",
            "Cnary variable has changed, but validator doesn't catch this error!"
        );
        *ptr--;
    }
    stackValidityCheck(
        stack,
        0,
        NULL,
        NULL
    );

    stackDest(stack);
    free(stack);
}

void test_changeData_rightSideAttack() ///< Тестируем валидатор, который должен отловить изменение канареечной переменной для массива данных
{
    Stack(int)* stack = (Stack(int)*)calloc(1, sizeof(Stack(int)));
    generateStack(stack);


    int errorCode = 0;
    {
        char* ptr = (char*)&stack->data + stack->capacity*stack->elementSize;
        *ptr++;
        stackValidityCheck(
            stack,
            STK_ERROR_ATTACK,
            "Edit canary cell in structure...",
            "Cnary variable has changed, but validator doesn't catch this error!"
        );
        *ptr--;
    }
    stackValidityCheck(
        stack,
        0,
        NULL,
        NULL
    );

    stackDest(stack);
    free(stack);
}


#endif
void stackTester() ///< Тестирующая функция стека, содержит в себе 8 различных тестов
{
    test_pushData();
    #ifndef NDEBUG
        test_changeStructure_editHash();
        test_changeStructure_editCapacity();
        test_changeStructure_leftSideAttack();
        test_changeStructure_rightSideAttack();
        test_changeData_editBits();
        test_changeData_swapElements();
        test_changeData_randomEditElemtnts();
    #endif
}

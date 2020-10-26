#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../Logger.h"

//#define NDEBUG

#define Assert_c(expr) if(!(expr)) loggerAssert(#expr,__FILE__,__FUNCSIG__,__LINE__);

#ifndef NDEBUG
    //#define STK_CANARY_PROTECTION           ///<�������� ������ �����������
    //#define STK_HASH_PROTECTION             ///<�������� ������ ������
#endif


/**
\brief ��������� ��������� � �������� ����������, �������� ������ ����������� 2 ������ ������ �����
*/
#ifdef STK_CANARY_PROTECTION
    #define ON_STK_CANARY_PROTECTION(code) code
#else
    #define ON_STK_CANARY_PROTECTION(code)
#endif

#ifdef STK_HASH_PROTECTION
    #define ON_STK_HASH_PROTECTION(code) code
#else
    #define ON_STK_HASH_PROTECTION(code)
#endif

/**
\brief ��� ��������� ���������� �������� ������ �������� ����� ����������� �����
@{
*/
typedef unsigned char ui8;
typedef unsigned int ui32;
typedef unsigned long long ui64;
ON_STK_CANARY_PROTECTION(typedef ui64 CanaryType);
ON_STK_HASH_PROTECTION(typedef ui64 Hash);
/**}@*/


/**
\brief ��������� ��� �������� ���������� ����������
*/
struct dbgCallInfo
{
    const char* file;
    const char* func;
    long        line;
    const char* varName;
};

/**
\brief ���������, ������������ ��������� �����
@{
*/
ON_STK_CANARY_PROTECTION(const CanaryType STK_CANARY_VALUE = 0x99996666AAAA5555);  ///< Magic number, ������� ������������ � ����������-���������, ��� ������������ ���� �� ��������� ����� � ������.
const ui8 STK_CANARY_SIZE = ON_STK_CANARY_PROTECTION(sizeof(CanaryType)) + 0;      ///< ������ ����������� ����������
const ui64 STK_BUFFER_ADDITION = 8;                                                ///< ���������, �� ������� �������������� ����� capacity, ����� ����, ��� ����� �������������
/**}@*/


/**
\brief ��������� ������
@{
*/
enum StackError { 
    STK_OK                          =  0,   ///< ������ �� ��������
    STK_ERROR_NULL_PTR              = -1,   ///< ������ ���������, ���� � ������� �������� ������� ���������
    STK_ERROR_OUT_OF_MEMORY         = -2,   ///< ������ ���������, ���� �� ������� �������� ������ calloc ��� �������� ������ ��� ������ realloc
    STK_ERROR_INVALID_PTR           = -3,   ///< ������ ���������, ���� ��������� �� ����������� ������ ��� data � ��������� �������� NULL
    STK_ERROR_OUT_OF_RANGE          = -4,   ///< ������ ���������, ���� ������ ����� ������, ��� ���������� ������
    STK_ERROR_ATTACK                = -5,   ///< ������ ���������, ���� ��������� ����� ��� ������ ���� ���������, ����������� ��������� ����������� ����������
    STK_ERROR_STK_IS_EMPTY          = -6,   ///< ������ ���������, ���� �� ������� ����� �������� �������� ������
    STK_ERROR_CHANGED_DATA          = -7,   ///< ������ ���������, ���� ������ ���� ��������, ����������� ����������� �������� ���� �� ������� ������
    STK_ERROR_CHANGED_STRUCTURE     = -8    ///< ������ ���������, ���� ��������� ����� ���� ���������, ����������� ����������� �������� ���� �� ��������� �����
};
/**}@*/


/**
\brief    ����������� ��������� �����, ������� �� �������� ���������� � ���� ������.
\detail   ���� ���������� ����� ����������� ����������� ������ ���������, ����� ���������
          ��������� ���������� ����� � ������ ����� ������.
*/
struct _BaseStack
{
    ON_STK_CANARY_PROTECTION(const CanaryType leftSide = STK_CANARY_VALUE);
    ON_STK_HASH_PROTECTION (Hash structHash);
    ON_STK_HASH_PROTECTION (Hash dataHash  );
    ui32 elementSize;
    ui32 size;
    ui32 capacity;
    void* data;
    ON_STK_CANARY_PROTECTION(const CanaryType rightSide = STK_CANARY_VALUE);
};


/**
\brief   ������� ��� ������ � ����������� ������.
@{
*/
StackError _stackInit(void* stk, const ui32 capacity, const ui32 elementSize);
StackError _stackValidity(const void* stk, const dbgCallInfo dbgInfo = {});
StackError _stackPush(void* stk, void* value, const dbgCallInfo dbgInfo = {});
StackError _stackPop(void* stk, void* dest, const dbgCallInfo dbgInfo = {});
void _stackDump(const void* stk, const dbgCallInfo dbgInfo, FILE* outStream = NULL);
void _stackDest(void* stk);
/**
}@
\note ��������� �������� ������ ������� ���� � ����� Stack.cpp
*/
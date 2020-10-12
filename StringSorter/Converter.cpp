#include "Converter.h"
#include <stdlib.h>
#include <stdio.h>
#include <clocale>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>


/**
\brief  Функция считает количество символов, закодированных в utf8
\param  [in]      src   Указатель на строку, закодированную в utf8
\return Возвращает количество символов, закодированных в utf8 или код ошибки.
\note   Ошибка может быть возвращена, если в строке имеется символ,
        для кодирования которого требуется более 3х байт.
*/
int utf8StrLen(const char* src)
{
    int length = 0;
    Assert_c(src != NULL);
    if (!src)
        return CONVERTER_ERR_NULL_PTR;
    const char* ptr = src;
    while (*ptr)
    {
        length++;
        char v = (*ptr);
        if (v >= 0)
        {
            ptr++;
            continue;
        }
        int shiftCount = 0;
        if ((v & 0xE0) == 0xC0)
            shiftCount = 1;
        else if ((v & 0xF0) == 0xE0)
            shiftCount = 2;
        else
            return CONVERTER_ERR_CANT_CONVERT;
        ptr+= shiftCount+1;
    }
    return length;
}


/**
\brief  Функция преобразвет строку, закодированную в utf8 в массив wchar_t
\param  [in,out]  dest  Указатель на преобразованный массив
\param  [in]      src   Указатель на строку, закодированную в utf8
\return Возвращает 0, если преобразование прошло успешно, в случае ошибки возвращается её код.
\note   Ошибка может возникнуть, если исходная utf8 строка содержала в себе символы,
        которые кодируются более чем 3 байтами, дело в том, что такие символы не
        помещаются в whcar_t.
*/
int utf8ToWchar(wchar_t* dest, const char* src)
{
    Assert_c(src != NULL);
    if (!src)
        return CONVERTER_ERR_NULL_PTR;
    Assert_c(dest != NULL);
    if (!dest)
        return CONVERTER_ERR_NULL_PTR;
    wchar_t* resultStr = dest;
    wchar_t c;
    const char* ptr = src;
    while (*ptr)
    {
        char v = (*ptr);
        if (v >= 0)
        {
            c = v;
            *resultStr = c;
            resultStr++;
            ptr++;
            continue;
        }
        int shiftCount = 0;
        if ((v & 0xE0) == 0xC0)
        {
            shiftCount = 1;
            c = v & 0x1F;
        }
        else if ((v & 0xF0) == 0xE0)
        {
            shiftCount = 2;
            c = v & 0xF;
        }
        else
            return CONVERTER_ERR_CANT_CONVERT;
        ptr++;

        while (shiftCount)
        {
            v = *ptr;
            ptr++;
            c <<= 6;
            c |= (v & 0x3F);
            shiftCount--;
        }
        *resultStr = c;
        resultStr++;
    }
    return 0;
}

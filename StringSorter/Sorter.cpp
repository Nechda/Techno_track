#include "Sorter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cwctype>
#include <clocale>




/**
\brief Структура, описывающая соответствие между подстроками
*/
struct Link
{
    wchar_t* start;           ///< начало сточки в преобразованной последовательности
    wchar_t* end;             ///< конец строчки в преобразованной последовательности
    wchar_t* original_start;  ///< начало строки, считанной из файла
};

/**
\brief Функция подсчета непустых строк в файле
\param [in]   filename    Имя файла, в котором требуется подсчитать количество строк
\return Количество строк в файле или код ошибки.
*/
int getNumberLines(const char* filename)
{
    Assert_c(filename != NULL);
    if (filename == NULL)
        return SS_ERROR_NULLPTR;

    FILE* file = fopen(filename, "r");
    Assert_c(file != NULL);
    if (file == NULL)
        return SS_ERROR_ACCESS_FAIL;


    int lines = 0;
    wchar_t w[2] = { 0,L'\n' };
    bool pingpong = 0;
    while ((w[pingpong] = fgetwc(file)) != WEOF)
    {
        if (w[pingpong] == L'\n' & w[pingpong ^ 1] != L'\n')
            lines++;
        pingpong ^= 1;
    }

    fclose(file);
    return lines;
}

/**
\brief Функция подсчитывает количество байт в файле, отлиных от L'\n'.
\param [in]  filename   Имя файла, в котором требуется посчитать количетсво байт
\return Количество байт в файле, отличных от L'\n' или код ошибки.
*/
int getSizeFile(const char* filename)
{

    Assert_c(filename != NULL);
    if (filename == NULL)
        return SS_ERROR_NULLPTR;

    FILE* file = fopen(filename, "r");
    Assert_c(file != NULL);
    if (file == NULL)
        return SS_ERROR_ACCESS_FAIL;

    int size = 0;

    wchar_t w = 0;
    while ((w = fgetwc(file)) != WEOF)
        if (w != L'\n')
            size++;
    fclose(file);
    return size;
}


/**
\brief Функция сравнения двух строк
\detail Данная функция сравнивает две строки. Направление сравнения может быть задано параметром inc.
        Если inc = 1, то в качестве указателей пределается начало строк.
        Если inc = -1, то в качестве указателя требуется предавать конец строки.
\param  [in]   ptr1    Первая строка
\param  [in]   ptr2    Вторая строка
\param  [in]   inc     Добавляемое значение
\retrun Возвращает положительное число, если первая строка идет позже (в смысле лексиграфического порядка),
        чем вторая строка. Соответсвенно возвращает отрицательное, если вторая строка идет позже первой,
        и ноль в случае совпадания строк.

\note Это просто расширенная реализация функции wcscmp(const wchar_t* ptr1,const wchar_t* ptr2)
*/
int compare_wstr(const void* ptr1, const void* ptr2,int inc)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);

    if (ptr1 == NULL || ptr2 == NULL)
        return 0;

    C_string s1 = *((C_string*)ptr1);
    C_string s2 = *((C_string*)ptr2);
    while (*s1 && (*s1 == *s2))
        s1+=inc, s2+=inc;

    return (unsigned int)(*s1) - (unsigned int)(*s2);
}


/**
\brief  Функция сравнения двух структур Bijection
\detail Данная функция производит сравнение двух строк, которые записаны в структуре Bijection
        в полях start и end. В зависимости от выбора функции будет использоваться тот или этой указатель
        в Bijection.
\param  [in]   ptr1    Указатель на первую структуру Bijection
\param  [in]   ptr2    Указатель на вторую структуру Bijection
\retrun Возвращает положительное число, если первая строка (из структуры Bijection) идет позже (в смысле лексиграфического порядка),
        чем вторая строка. Соответсвенно возвращает отрицательное, если вторая строка идет позже первой,
        и ноль в случае совпадания строк.
@{
*/
static int compare_bijection_direct(const void* ptr1, const void* ptr2)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);
    if (ptr1 == NULL || ptr2 == NULL)
        return 0;

    Link b1 = *((Link*)ptr1);
    Link b2 = *((Link*)ptr2);
    return compare_wstr(&b1.start,&b2.start, 1);
}
static int compare_bijection_inverse(const void* ptr1, const void* ptr2)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);
    if (ptr1 == NULL || ptr2 == NULL)
        return 0;

    Link b1 = *((Link*)ptr1);
    Link b2 = *((Link*)ptr2);
    return compare_wstr(&b1.end, &b2.end, -1);
}
/** @}*/

/**
\brief  Функция генерирует массив символов, содержащий строки считываемого файла
\detail Функция генерирует массив символов, содержащий строки считываемого файла.
         Каждая строка в массиве отделена от предыдущей нулевым символом. Пустые 
         строки в массив не записываются.
\param  [in]      filename          Имя считываемого файла
\param  [in,out]  out_data          Выходной массив
\param  [out]     array_len         Размер выходного массива
\param  [out]     num_dividers      Количество разделителей в массиве

*/
static int generateDividedString(const char* filename, C_string* out_data, int* array_len = NULL, int* num_dividers = NULL)
{
    Assert_c(filename != NULL);
    if (filename == NULL)
        return SS_ERROR_NULLPTR;

    FILE* file = fopen(filename, "r");
    Assert_c(file != NULL);
    if (file == NULL)
        return SS_ERROR_ACCESS_FAIL;

    int numLines = getNumberLines(filename);
    Assert_c(numLines >= 0);
    if (numLines < 0)
        return SS_INVALID_DATA;

    int sizeFile = getSizeFile(filename);
    Assert_c(sizeFile >= 0);
    if (sizeFile < 0)
        return SS_INVALID_DATA;
    
    Assert_c(out_data != NULL);
    if (!out_data)
        return SS_ERROR_NULLPTR;

    C_string data = (C_string)calloc(sizeFile + numLines, sizeof(C_string));
    Assert_c(data != NULL);
    if (!data)
        return SS_ERROR_NULLPTR;

    *out_data = data;


    wchar_t w[2] = { 0, L'\n' };
    bool pingpong = 0;
    while ((w[pingpong] = fgetwc(file)) != WEOF)
    {
        if (w[pingpong] != L'\n')
        {
            *data = w[pingpong];
            data++;
        }
        if (w[pingpong] == L'\n' && w[pingpong ^ 1] != L'\n')
        {
            *data = L'\0';
            data++;
        }
        pingpong ^= 1;
    }
    *data = L'\0';

    fclose(file);
    if (array_len)
        *array_len = sizeFile + numLines;
    if (num_dividers)
        *num_dividers = numLines;
    return 0;
}


/**
\brief  Функция считает количество знаков препинания в массиве
\param  [in]      in_data           Входной массив
\param  [in]      size_input_data   Размер входного массива
\return Возвращает количество знаков препинания в массиве или код ошибки.
*/
static inline int countSpecialSybmols(const C_string* in_data, const unsigned int size_input_data)
{
    Assert_c(in_data != NULL);
    Assert_c(*in_data != NULL);
    if (!in_data || !*in_data)
        return SS_ERROR_NULLPTR;
    C_string data = *in_data;
    

    int num_special_symbols = 0;
    for (int i = 0; i < size_input_data; i++)
        if (!iswalpha(data[i]) && data[i])
            num_special_symbols++;
    return num_special_symbols;
}

/**
\brief  Функция генерирует массив без знаков препинания
\param  [in]      in_data           Исходный массив
\param  [in]      array_len         Длина входящего массива
\param  [in]      num_dividers      Количество разделителей в исходном массиве
\param  [in,out]  out_data          Выходной массив
\note   Возвращаемый указатель out_data на самом деле указывает не на нулевой элемент памяти, выделенной calloc,
        а на первый. Это связано с тем, что дальнейшая работа с этим массивом предполагает возможность чтения строк в 
        обратном порядке. Соответственно необходим нулевой символ, клоторый будет ограничиывать строку. Данный символ
        располагается перед массивом. При очистке требуется вызывать free(ptr-1);
\return Код ошибки или ноль в случае успеха.
*/
static int generatePureString(const C_string* in_data, const int array_len, const int num_dividers, C_string* out_data)
{
    Assert_c(in_data != NULL);
    Assert_c(out_data != NULL);
    if (in_data == NULL || out_data == NULL)
        return SS_ERROR_NULLPTR;

    
    int num_special_symbols = countSpecialSybmols(in_data, array_len);
    Assert_c(num_special_symbols >= 0);
    if (num_special_symbols < 0)
        return num_special_symbols;

    C_string data = *in_data;
    C_string transformed_data = (C_string)calloc(array_len - num_special_symbols + 1, sizeof(C_string));
    if (!transformed_data)
        return SS_ERROR_NULLPTR;
    *transformed_data = L'\0';
    transformed_data++;
    *out_data = transformed_data;

    Assert_c(data != NULL);
    if (!data)
        return SS_ERROR_NULLPTR;

    for (int i = 0; i < array_len; i++)
        if (std::iswalpha(data[i]) || !data[i])
        {
            *transformed_data = data[i];
            transformed_data++;
        }
    return 0;
}

/**
\brief  Функция генерит таблицу соответствий двух массивов
\param  [in]      in_data_first     Первый массив
\param  [in]      in_data_second    Второй массив
\param  [in]      num_dividers      Количество разделителей в первом массиве
\param  [in,out]  out_arr           Таблица соответствий 
\return Код ошибки или ноль в случае успеха.
*/
static int generateBijection(const C_string* in_data_first, const C_string* in_data_second, const int num_dividers, Link** out_arr)
{
    Assert_c(in_data_first != NULL);
    Assert_c(in_data_second != NULL);
    Assert_c(out_arr != NULL);
    Assert_c(*in_data_first != NULL);
    Assert_c(*in_data_second != NULL);
    if (!in_data_first || !in_data_second || !out_arr || !*in_data_first || !*in_data_second)
        return SS_ERROR_NULLPTR;

    Link* ranges = (Link*)calloc(num_dividers, sizeof(Link));;
    Assert_c(ranges != NULL);
    if(!ranges)
        return SS_ERROR_NULLPTR;
    *out_arr = ranges;

    ranges->original_start = 0;
    ranges->start = 0;
    ranges->end = 0;
    C_string ptr[2] = { *in_data_first,*in_data_second };
    for (int i = 0; i < num_dividers; i++)
    {
        ranges->original_start = ptr[0];
        ranges->start = ptr[1];

        ptr[0] = wcschr(ptr[0], L'\0');
        ptr[1] = wcschr(ptr[1], L'\0');
        Assert_c(ptr[0] != NULL);
        Assert_c(ptr[1] != NULL);
        if (!ptr[0] || !ptr[1])
            return SS_ERROR_NULLPTR;

        ;;
        ranges->end = ptr[1] - 1;

        ptr[0]++, ptr[1]++;
        ranges++;
    }
    return 0;
}

/**
\brief  Функция сортировки строк из файла
\detail Данная функция сортирует строки, считываемых из файла in_filename, и записывает результат в out_filename.
        Имеется возможность сравнивать строки с конца, для этого используется параметр isDirect.
        Если isDirect == true, то буквы стоящие в начале строки имеют больший приоритет, чем последующие.
        Если isDirect == false, то значимость букв увеличивается по мере приближения к концу строки.
\params [in]      in_filename       Имя входного файла
\params [in]      out_filename      Имя выходного файла
\params [in]      dir               Флаг, задающий направление сортировки строк

        Краткое описание алгоритма:
        1. На основе считанных из файла строк формируется одна строка STR1, в которой каждая строка исходного файла
           отделена от других строк нулевым символом.
        2. Заводится новая строка STR2, в которую копируются символы из STR1,кроме знаков препинания.
        3. Строится таблица соответствий между подстроками в STR1 и STR2.
        4. Производится сортировка данной таблицы, в которой сравниваются подстроки STR2.
        5. По отсортированной таблице соответствий восстанавливаем подстроки STR1 со знаками препинания.
        6. Результат записываем в выходной файл.
*/
void sort_FromFile(const char* in_filename, const char* out_filename,Direction dir)
{
    std::setlocale(LC_ALL, "en_US.utf8");

    Assert_c(in_filename != NULL);
    Assert_c(out_filename != NULL);
    if (in_filename == NULL || out_filename == NULL)
        return;

    FILE* file_out = fopen(out_filename, "w");
    Assert_c(file_out != NULL);
    if (file_out == NULL)
        return;

    int error_code = 0;

    
    //Построение строки, которая разделяет строки исходного файла нулевым символом
    int array_len = 0;
    int num_dividers = 0;
    C_string data = NULL;
    error_code = generateDividedString(in_filename, &data, &array_len, &num_dividers);
    if (error_code)
    {
        fclose(file_out);
        if (data)
            free(data);
        return;
    }


    //Удаление всех лишних символов
    C_string pure_str = NULL;
    error_code = generatePureString(&data, array_len, num_dividers, &pure_str);
    if (error_code)
    {
        fclose(file_out);
        free(data);
        if (pure_str)
            free(pure_str-1);
        return;
    }


   
    //Построение таблицы соответсвий
    Link* ranges = NULL;
    error_code = generateBijection(&data,&pure_str, num_dividers,&ranges);
    if (error_code)
    {
        fclose(file_out);
        free(data);
        free(pure_str-1);
        if (ranges)
            free(ranges);
        return;
    }

    //сортировка и вывод
    qsort(ranges, num_dividers, sizeof(Link), dir == Direct ? compare_bijection_direct : compare_bijection_inverse);

    for (int i = 0; i < num_dividers; i++)
        fprintf(file_out, "%d: %ls\n", i + 1, ranges[i].original_start);

    fclose(file_out);
    free(data);
    free(pure_str-1);
    free(ranges);
    return;
}
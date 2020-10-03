#include "Sorter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cwctype>
#include <clocale>
#include <ctime>
#include <ctype.h>
#include <cmath>


/**

TODO:
Переписать параметры в документации!!!!

**/


/**
\brief Структура, описывающая соответствие между подстроками
*/
struct Link
{
    wchar_t* start;           ///< начало сточки в преобразованной последовательности
    wchar_t* end;             ///< конец строчки в преобразованной последовательности
    wchar_t* originalStart;   ///< начало строки, считанной из файла
    wchar_t  rythmType;       ///< тип рифмы 
    wchar_t  nVowel;          ///< количество гласных
};

/**
\brief Функция генерации случайного целого числа в заданно диапазоне [a,b]
\param  [in]  a  Первое число
\param  [in]  b  Второе число
\return Случайное целое число из отрезка [a,b]
*/
inline int randInt(int a, int b)
{
    return rand() % (b - a + 1) + a;
}

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
int cmpWstr(const void* ptr1, const void* ptr2,int inc)
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
\brief  Функция сравнения двух структур Link
\detail Данная функция производит сравнение двух строк, которые записаны в структуре Link
        в полях start и end. В зависимости от выбора функции будет использоваться тот или этой указатель
        в Link.
\param  [in]   ptr1    Указатель на первую структуру Link
\param  [in]   ptr2    Указатель на вторую структуру Link
\retrun Возвращает положительное число, если первая строка (из структуры Bijection) идет позже (в смысле лексиграфического порядка),
        чем вторая строка. Соответсвенно возвращает отрицательное, если вторая строка идет позже первой,
        и ноль в случае совпадания строк.
@{
*/
static int cmpTableDirect(const void* ptr1, const void* ptr2)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);
    if (ptr1 == NULL || ptr2 == NULL)
        return 0;

    Link b1 = *((Link*)ptr1);
    Link b2 = *((Link*)ptr2);
    return cmpWstr(&b1.start,&b2.start, 1);
}
static int cmpTableInverse(const void* ptr1, const void* ptr2)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);
    if (ptr1 == NULL || ptr2 == NULL)
        return 0;

    Link b1 = *((Link*)ptr1);
    Link b2 = *((Link*)ptr2);
    return cmpWstr(&b1.end, &b2.end, -1);
}
/** @}*/

/**
\brief  Функция генерирует массив символов, содержащий строки считываемого файла.
\detail Функция генерирует массив символов, содержащий строки считываемого файла.
        Каждая строка в массиве отделена от предыдущей нулевым символом. Пустые 
        строки в массив не записываются.
\param  [in]      filename          Имя считываемого файла
\param  [in,out]  outArray          Выходной массив
\param  [out]     outArrayLength    Размер выходного массива
\param  [out]     outNumDividers    Количество разделителей в выходном массиве
\note  В массив записываются знаки препинания и русские буквы, остальные символы пропускаются.
*/
static int generateDividedString(const char* filename, C_string* outArray, int* outArrayLength = NULL, int* outNumDividers = NULL)
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
    
    Assert_c(outArray != NULL);
    if (!outArray)
        return SS_ERROR_NULLPTR;

    C_string data = (C_string)calloc(sizeFile + numLines, sizeof(C_string));
    Assert_c(data != NULL);
    if (!data)
        return SS_ERROR_NULLPTR;

    *outArray = data;


    wchar_t w[2] = { 0, L'\n' };
    bool pingpong = 0;
    
    sizeFile = 0;
    numLines = 0;

    while ((w[pingpong] = fgetwc(file)) != WEOF)
    {
        bool isValidSymbol = (bool)wcschr(L",.;?! ", w[pingpong]) | (L'А' <= w[pingpong] && L'Я' >= w[pingpong] ) || (L'а' <= w[pingpong] && L'я' >= w[pingpong]);
        if (w[pingpong] != L'\n' & isValidSymbol)
        {
            *data = w[pingpong];
            data++;
            sizeFile++;
        }
        if (w[pingpong] == L'\n' && w[pingpong ^ 1] != L'\n')
        {
            *data = L'\0';
            data++;
            numLines++;
        }
        pingpong ^= 1;
    }
    *data = L'\0';
    numLines++;

    fclose(file);
    if (outArrayLength)
        *outArrayLength = sizeFile + numLines;
    if (outNumDividers)
        *outNumDividers = numLines;
    return 0;
}


/**
\brief  Функция считает количество знаков препинания в массиве
\param  [in]      inArray           Входной массив
\param  [in]      arrLength         Размер входного массива
\return Возвращает количество знаков препинания в массиве или код ошибки.
*/
static inline int countSpecialSybmols(const C_string* inArray, const unsigned int arrLength)
{
    Assert_c(inArray != NULL);
    if (!inArray)
        return SS_ERROR_NULLPTR;
    Assert_c(*inArray != NULL);
    if (!*inArray)
        return SS_ERROR_NULLPTR;
    C_string data = *inArray;
    

    int num_special_symbols = 0;
    for (int i = 0; i < arrLength; i++)
        if (!iswalpha(data[i]) && data[i])
            num_special_symbols++;
    return num_special_symbols;
}

/**
\brief  Функция генерирует массив без знаков препинания
\param  [in]      inArray           Исходный массив
\param  [in]      arrLength         Длина входящего массива
\param  [in]      nDividers         Количество разделителей в исходном массиве
\param  [in,out]  outArray          Выходной массив
\note   Возвращаемый указатель outArray на самом деле указывает не на нулевой элемент памяти, выделенной calloc,
        а на первый. Это связано с тем, что дальнейшая работа с этим массивом предполагает возможность чтения строк в 
        обратном порядке. Соответственно необходим нулевой символ, клоторый будет ограничиывать строку. Данный символ
        располагается перед массивом. При очистке требуется вызывать free(ptr-1);
\return Код ошибки или ноль в случае успеха.
*/
static int generatePureString(const C_string* inArray, const int arrLength, const int nDividers, C_string* outArray)
{
    Assert_c(inArray != NULL);
    Assert_c(outArray != NULL);
    if (inArray == NULL || outArray == NULL)
        return SS_ERROR_NULLPTR;

    
    int num_special_symbols = countSpecialSybmols(inArray, arrLength);
    Assert_c(num_special_symbols >= 0);
    if (num_special_symbols < 0)
        return num_special_symbols;

    C_string data = *inArray;
    C_string transformed_data = (C_string)calloc(arrLength - num_special_symbols + 1, sizeof(C_string));
    if (!transformed_data)
        return SS_ERROR_NULLPTR;
    *transformed_data = L'\0';
    transformed_data++;
    *outArray = transformed_data;

    Assert_c(data != NULL);
    if (!data)
        return SS_ERROR_NULLPTR;

    for (int i = 0; i < arrLength; i++)
        if (std::iswalpha(data[i]) || !data[i])
        {
            *transformed_data = data[i];
            transformed_data++;
        }
    return 0;
}

/**
\brief  Функция генерит таблицу соответствий двух массивов
\param  [in]      firstArray     Первый массив
\param  [in]      secondArray    Второй массив
\param  [in]      nDividers      Количество разделителей в первом массиве
\param  [in,out]  outTable       Указатель на таблицу соответствий
\note   Память под таблицу выделять не нужно. Всю память функция выделяет автоматически.
        Требуется только указать, где будет храниться указатель на таблицу, именно это значение и
        будет записано в outTable.
\return Код ошибки или ноль в случае успеха.
*/
static int generateBijection(const C_string* firstArray, const C_string* secondArray, const int nDividers, Link** outTable)
{
    Assert_c(firstArray != NULL);
    Assert_c(secondArray != NULL);
    Assert_c(outTable != NULL);
    if (!firstArray || !secondArray || !outTable)
        return SS_ERROR_NULLPTR;
    Assert_c(*firstArray != NULL);
    Assert_c(*secondArray != NULL);
    if(!*firstArray || !*secondArray)
        return SS_ERROR_NULLPTR;
    
    Link* table = (Link*)calloc(nDividers, sizeof(Link));;
    Assert_c(table != NULL);
    if(!table)
        return SS_ERROR_NULLPTR;
    *outTable = table;

    table->originalStart = 0;
    table->start = 0;
    table->end = 0;
    C_string ptr[2] = { *firstArray,*secondArray };
    for (int i = 0; i < nDividers; i++)
    {
        table->originalStart = ptr[0];
        table->start = ptr[1];

        ptr[0] = wcschr(ptr[0], L'\0');
        ptr[1] = wcschr(ptr[1], L'\0');
        Assert_c(ptr[0] != NULL);
        Assert_c(ptr[1] != NULL);
        if (!ptr[0] || !ptr[1])
            return SS_ERROR_NULLPTR;

        ;;
        table->end = ptr[1] - 1;

        ptr[0]++, ptr[1]++;
        table++;
    }
    return 0;
}

/**
\brief  Функция сортировки строк из файла
\detail Данная функция сортирует строки, считываемых из файла in_filename, и записывает результат в out_filename.
        Имеется возможность сравнивать строки с конца, для этого используется параметр isDirect.
        Если dir == Direct, то буквы стоящие в начале строки имеют больший приоритет, чем последующие.
        Если dir == Inverse, то значимость букв увеличивается по мере приближения к концу строки.
\params [in]      inFilename       Имя входного файла
\params [in]      outFilename      Имя выходного файла
\params [in]      dir              Флаг, задающий направление сортировки строк
        Краткое описание алгоритма:
        1. На основе считанных из файла строк формируется одна строка STR1, в которой каждая строка исходного файла
           отделена от других строк нулевым символом.
        2. Заводится новая строка STR2, в которую копируются символы из STR1,кроме знаков препинания.
        3. Строится таблица соответствий между подстроками в STR1 и STR2.
        4. Производится сортировка данной таблицы, в которой сравниваются подстроки STR2.
        5. По отсортированной таблице соответствий восстанавливаем подстроки STR1 со знаками препинания.
        6. Результат записываем в выходной файл.
*/
void sortFromFile(const char* inFilename, const char* outFilename,Direction dir)
{
    std::setlocale(LC_ALL, "en_US.utf8");

    Assert_c(inFilename != NULL);
    Assert_c(outFilename != NULL);
    if (inFilename == NULL || outFilename == NULL)
        return;

    FILE* outFile = fopen(outFilename, "w");
    Assert_c(outFile != NULL);
    if (outFile == NULL)
        return;

    int errorCode = 0;

    
    //Построение строки, которая разделяет строки исходного файла нулевым символом
    int arrLength = 0;
    int nDividers = 0;
    C_string data = NULL;
    errorCode = generateDividedString(inFilename, &data, &arrLength, &nDividers);
    if (errorCode)
    {
        fclose(outFile);
        if (data)
            free(data);
        return;
    }


    //Удаление всех лишних символов
    C_string pureStr = NULL;
    errorCode = generatePureString(&data, arrLength, nDividers, &pureStr);
    if (errorCode)
    {
        fclose(outFile);
        free(data);
        if (pureStr)
            free(pureStr-1);
        return;
    }


   
    //Построение таблицы соответсвий
    Link* table = NULL;
    errorCode = generateBijection(&data,&pureStr, nDividers,&table);
    if (errorCode)
    {
        fclose(outFile);
        free(data);
        free(pureStr-1);
        if (table)
            free(table);
        return;
    }

    //сортировка и вывод
    qsort(table, nDividers, sizeof(Link), dir == Direct ? cmpTableDirect : cmpTableInverse);

    for (int i = 0; i < nDividers; i++)
        if(std::iswalpha(table[i].originalStart[0]))
            fprintf(outFile, "%ls\n", table[i].originalStart);

    fclose(outFile);
    free(data);
    free(pureStr-1);
    free(table);
    return;
}



///=====================================================================================================///
///=====================================================================================================///



/**
\brief  Функция сравнения элементов таблицы, с учетом типа литературного слога
\param  [in]  ptr1   Указатель на первый элемент таблицы
\param  [in]  ptr2   Указатель на второй элемент таблицы
\return В случае, если первый элемент таблицы должен идти раньше возвращается положительное число.
        В случае, если первый элемент таблицы должен идти позже второго, возвращается отрицательное число.
        Если элементы таблицы равны, то возвращается 0.
*/
static int cmpSyllable(const void* ptr1, const void* ptr2)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);
    if (ptr1 == NULL || ptr2 == NULL)
        return 0;

    Link b1 = *((Link*)ptr1);
    Link b2 = *((Link*)ptr2);

    int cmp = b1.rythmType - b2.rythmType;
    if (!cmp)
        cmp = cmpWstr(&b1.end, &b2.end, -1);
    return cmp;
}

/**
\brief  Структура, реализующая циклический указатель
\detail Данная структура реализует циклический указатель. Его суть состоит в том,
        что при вызове функции GetPtr(...) значение ptr в структуре будет инкреминтеровано, причем
        достижение границы автоматически переведет указатель в самое начало.
@{
*/
struct RangedPtr
{
    int start;
    int end;
    int ptr;
};

/// Функция, инкрементирования указателя
static int getPtr(RangedPtr* rptr,int inc = 1) 
{
    if (rptr->ptr < rptr->start)
        rptr->ptr = rptr->start;
    while (inc)
    {
        rptr->ptr++;
        if (rptr->end < rptr->ptr)
            rptr->ptr = rptr->start;
        inc--;
    }
    return rptr->ptr;
}
/** @}*/

/**
\brief Функция находит интервал ячеек в отсортированной таблице соответствий, с одинаковым окончанием.
\param  [in]     ptrTable  Указатель на таблицу соответствий
\param  [in]     arrSize   Количество элеметов в таблице
\param  [in,out] range     Циклический указатель, в котором будет содержаться интервал
\note   Для корректной работы функции требуется, чтобы в циклическом указателе были заданы параметры
        начала и конца, равные между собой. Именно от этого значения будет расширяться интервал.
*/
static inline void findRanges(Link** ptrTable, const int arrSize,RangedPtr* range)
{

    Assert_c(ptrTable != NULL);
    if (ptrTable == NULL)
        return;
    Assert_c(*ptrTable != NULL);
    if (ptrTable == NULL)
        return;
    Assert_c(range != NULL);
    if (!range)
        return;

    Assert_c(range->start == range->end);
    if (range->start != range->end)
        return;

    const Link* table = *ptrTable;
    int* ptr = &range->start;

#define prev( addr ) *(addr-1)
#define curr( addr ) *(addr)
    wchar_t w[2] = { prev(table[ptr[0]].end), curr(table[ptr[0]].end) };

    while ( ptr[1] < arrSize
        &&  prev(table[ptr[1]].end) == w[0]
        &&  curr(table[ptr[1]].end) == w[1] )
        ptr[1] ++;
    ptr[1] --;

    while ( 0 <= ptr[0]
        &&  prev(table[ptr[0]].end) == w[0]
        &&  curr(table[ptr[0]].end) == w[1] )
        ptr[0] --;
    ptr[0] ++;
#undef prev
#undef curr

}


/**
\brief  Функция генерирует некоторое количество строф, основываясь на форматирующей строке.
\detail Функция генерирует некоторое количество строф, основываясь на форматирующей строке. 
        Для генерации требуется словарь риф, а так же его его структура. Результат записывается в файл.
\param  [in]  ptrTable       Указатель на таблицу соответствий
\param  [in]  arrSize        Размер таблицы
\param  [in]  inSyllableStr  Строка, определяющя типы рифм в словаре
\param  [in]  outFilename    Имя выходного файла
\param  [in]  outSyllableStr Строка, определяющая структуру рифм в выходном файле
\param  [in]  nStanzas       Число генерируемых строф
*/
static void stanzaGenerator(Link** ptrTable, const int arrSize, const char* inSyllableStr, const char* outFilename, const char* outSyllableStr,const int nStanzas = 1)
{
    srand(time(0));
    Assert_c(ptrTable != NULL);
    if (!ptrTable)
        return;
    Assert_c(*ptrTable != NULL);
    if (!*ptrTable)
        return;
    Assert_c(inSyllableStr != NULL);
    if (!inSyllableStr)
        return;
    Assert_c(outFilename != NULL);
    if (!outFilename)
        return;
    Assert_c(outSyllableStr != NULL);
    if (!outSyllableStr)
        return;


    FILE* poemFile = fopen(outFilename, "w");
    Assert_c(poemFile != NULL);
    if (!poemFile)
        return;

    int inSyllableLength = strlen(inSyllableStr);
    int outSyllableLength = strlen(outSyllableStr);
    char* inSyllable = (char*)calloc(inSyllableLength, sizeof(char));
    char* outSyllable = (char*)calloc(outSyllableLength, sizeof(char));
    Assert_c(inSyllable != NULL);
    Assert_c(outSyllable != NULL);
    if (!inSyllable || !outSyllable)
    {
        if (inSyllable)
            free(inSyllable);
        if (outSyllable)
            free(outSyllable);
        fclose(poemFile);
        return;
    }

    for (int i = 0; i < inSyllableLength; i++)
        inSyllable[i] = tolower(inSyllableStr[i]);
    for (int i = 0; i < outSyllableLength; i++)
        outSyllable[i] = tolower(outSyllableStr[i]);

    //вычисляем количество требуемых видов окончаний
    int nEdings = 0;
    for (int i = 0; i < outSyllableLength; i++)
        nEdings = nEdings < outSyllable[i] ? outSyllable[i] : nEdings;
    nEdings -= 'a';
    nEdings++;

    Link* table = *ptrTable;
    RangedPtr* ryhmInterval = (RangedPtr*)calloc(nEdings, sizeof(RangedPtr));
    Assert_c(ryhmInterval != NULL);
    if (!ryhmInterval)
    {
        free(inSyllable);
        free(outSyllable);
        fclose(poemFile);
        return;
    }

    for (int j = 0; j < nEdings; j++)
    {
        ryhmInterval[j].start = arrSize;
        ryhmInterval[j].end = 0;
    }

#define max(a,b) a>b ? a:b
#define min(a,b) a>b ? b:a
    for (int j = 0; j < nEdings; j++)
    {
        for (int i = 0; i < arrSize; i++)
            if (table[i].rythmType == j + 1)
            {
                ryhmInterval[j].start = min(ryhmInterval[j].start, i);
                ryhmInterval[j].end = max(ryhmInterval[j].end, i);
            }
    }
#undef max
#undef min



    RangedPtr* syllableInterval = (RangedPtr*)calloc(nEdings, sizeof(RangedPtr));
    Assert_c(syllableInterval != NULL);
    if (!syllableInterval)
    {
        free(syllableInterval);
        free(inSyllable);
        free(outSyllable);
        fclose(poemFile);
        return;
    }


    for (int nStanza = 0; nStanza < nStanzas; nStanza++)
    {

        for (int j = 0; j < nEdings; j++)
        {
            do
            {
                syllableInterval[j].start = randInt(ryhmInterval[j].start, ryhmInterval[j].end);
                syllableInterval[j].end = syllableInterval[j].start;
                findRanges(&table, arrSize, &syllableInterval[j]);
            } while (syllableInterval[j].start == syllableInterval[j].end);
            syllableInterval[j].ptr = randInt(syllableInterval[j].start, syllableInterval[j].end);   
        }


        for (int i = 0; i < outSyllableLength; i++)
        {
            int index = getPtr(&syllableInterval[outSyllable[i] - 'a'], randInt(1,4));
            fprintf(poemFile, "%ls\n", table[index].originalStart);
        }
        fprintf(poemFile, "\n\n\n");
    }

    free(syllableInterval);
    free(inSyllable);
    free(outSyllable);
    fclose(poemFile);
}


/**
\brief  Функция реализует бредогенератор
\detail Функция генерирует заданное количество строф, опираясь на форматирующую строку и словарь рифм.
\param  [in]  inFilename     Имя файла, из которого будут считываться строки
\param  [in]  inSyllableStr  Форматирующая строка, задающая тип строфы, используемой в оригинальном произведении
\param  [in]  outFilename    Имя файла, в который будет записываться результат генерации
\param  [in]  outSyllableStr Форматирующая строка, задающая тип строфы, используемой в выходном стихотворении
\param  [in]  nStanzas       Количество генерируемых строф

\note   Данная функция берет на вход файл, содержащий текст некоторого стихотворного произведения. В форматирующей
        строке должна быть записана структура строфы, используемой в исходном произведении. Например, для Евгения Онегина
        требуется передать в качестве параметра inSyllableStr строку вида "AbAbCCddEffEgg". Подробнее о формировании таких строк
        можно почитать в инете по запросу "рифменная схема". Аналогичного типа строка должна быть записана в outSyllableStr.
*/
void poermGenerator(const char* inFilename,const char* inSyllableStr, const char* outFilename,const char* outSyllableStr,const int nStanzas)
{

    std::setlocale(LC_ALL, "en_US.utf8");

    Assert_c(inFilename != NULL);
    Assert_c(outFilename != NULL);
    if (inFilename == NULL || outFilename == NULL)
        return;

    FILE* outFile = fopen(outFilename, "w");
    Assert_c(outFile != NULL);
    if (outFile == NULL)
        return;

    int errorCode = 0;


    //Построение строки, которая разделяет строки исходного файла нулевым символом
    int arrLength = 0;
    int numDividers = 0;
    C_string data = NULL;
    errorCode = generateDividedString(inFilename, &data, &arrLength, &numDividers);
    if (errorCode)
    {
        fclose(outFile);
        if (data)
            free(data);
        return;
    }


    //Удаление всех лишних символов
    C_string pureStr = NULL;
    errorCode = generatePureString(&data, arrLength, numDividers, &pureStr);
    if (errorCode)
    {
        fclose(outFile);
        free(data);
        if (pureStr)
            free(pureStr - 1);
        return;
    }



    //Построение таблицы соответсвий
    Link* table = NULL;
    errorCode = generateBijection(&data, &pureStr, numDividers, &table);
    if (errorCode)
    {
        fclose(outFile);
        free(data);
        free(pureStr - 1);
        if (table)
            free(table);
        return;
    }



    //инициализация входных слоговых конструкций
    {
        int index = 0;
        int inSyllableLength = strlen(inSyllableStr);
        for (int i = 0; i < numDividers; i++)
        {
            if (std::iswalpha(table[i].start[0]) && std::iswlower(table[i].start[1]))
            {
                table[i].rythmType = tolower(inSyllableStr[index]) - 'a' + 1;
                index++;
                index %= inSyllableLength;
            }
            else
                table[i].rythmType = 0;


        }
    }

    //составление словаря рифм
    qsort(table, numDividers, sizeof(Link), cmpSyllable);

    //генерируем бред в файл
    stanzaGenerator(&table, numDividers, inSyllableStr, outFilename, outSyllableStr, nStanzas);

    free(data);
    free(pureStr - 1);
    free(table);
    return;
}
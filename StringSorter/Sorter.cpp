#include "Sorter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cwctype>
#include <clocale>
#include <ctime>
#include <ctype.h>
#include <cmath>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "Converter.h"




/**
\brief Структура, описывающая соответствие между подстроками
*/
struct Link
{
    wchar_t* start;               ///< начало строки, считанной из файла
    wchar_t* end;                 ///< начало строки, считанной из файла
    wchar_t  rythmType;           ///< тип рифмы 
    wchar_t  nVowel;              ///< количество гласных
    short    ending;              ///< Номер окончания
    bool     isPenultimateVowel;  ///< является ли предполсденяя буква гласной
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
\brief Функция сравнения двух строк, игнорирующая знаки препинания
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
int cmpWstr(const void* ptr1, const void* ptr2, int inc)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);

    if (ptr1 == NULL || ptr2 == NULL)
        return 0;

    C_string s1 = *((C_string*)ptr1);
    C_string s2 = *((C_string*)ptr2);

    while (!std::iswalpha(*s1) && *s1)
        s1 += inc;

    while (!std::iswalpha(*s2) && *s2)
        s2 += inc;

    while (*s1 && (*s1 == *s2))
    {
        do
            s1 += inc;
        while (!std::iswalpha(*s1) && *s1);
           
        do
            s2 += inc;
        while (!std::iswalpha(*s2) && *s2);
    }

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
    return cmpWstr(&b1.start, &b2.start, 1);
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
       Символ, стоящий перед указателем outArray всегда будет нулевым.
       При освобождении памяти следует вызывать free(outArray-1);
*/
static int generateDividedString(const char* filename, C_string* outArray, int* outArrayLength = NULL, int* outNumDividers = NULL)
{
    Assert_c(filename != NULL);
    if (filename == NULL)
        return SS_ERROR_NULLPTR;

    int fileSize = 0;
    int numLines = 0;
    {
        struct stat st;
        stat(filename, &st);
        fileSize = st.st_size;
    }

    int filedesc = open(filename, O_RDONLY);
    if (filedesc == -1)
        return SS_ERROR_ACCESS_FAIL;

    char* buffer = (char*)calloc(fileSize + 2, sizeof(char));
    if (!buffer)
        return SS_ERROR_NULLPTR;
    buffer++;
    buffer[fileSize] = 0;
    buffer[-1] = 0;
    

    int nReadBytes = read(filedesc, buffer, fileSize);
    if (nReadBytes< 0)
    {
        free(buffer - 1);
        close(filedesc);
        return SS_ERROR_ACCESS_FAIL;
    }
    buffer[nReadBytes] = 0;

    int countLetters = utf8StrLen(buffer);
    C_string data = (C_string)calloc(countLetters + 2, sizeof(C_string));
    data[0] = 0;
    data++;
    data[countLetters] = 0;
    utf8ToWchar(data, buffer);

    free(buffer-1);
    close(filedesc);

    *outArray = data;
    
    wchar_t w[2] = { 0, L'\n' };
    bool pingpong = 0;
    int currPtr = 0;
    int additionPtr = 0;

    fileSize = 0;
    numLines = 0;
    

    while ((w[pingpong] = data[additionPtr]) != 0)
    {
        bool isValidSymbol = (bool)wcschr(L",.;?!- \"", w[pingpong]) | (L'А' <= w[pingpong] && L'Я' >= w[pingpong]) || (L'а' <= w[pingpong] && L'я' >= w[pingpong]);
        if (w[pingpong] != L'\n' & isValidSymbol)
        {
            data[currPtr] = w[pingpong];
            currPtr++;
            fileSize++;
        }
        if (w[pingpong] == L'\n' && w[pingpong ^ 1] != L'\n')
        {
            data[currPtr] = L'\0';
            currPtr++;
            numLines++;
        }
        
        pingpong ^= 1;
        additionPtr++;
    }
    data[currPtr] = 0;
    numLines++;
    

    data = (C_string)realloc(data - 1, (fileSize + numLines +  1)*sizeof(wchar_t));
    Assert_c(data != NULL);
    if (!data)
        return SS_ERROR_NULLPTR;

    if (outArrayLength)
        *outArrayLength = fileSize + numLines;
    if (outNumDividers)
        *outNumDividers = numLines;
    return 0;
}

/**
\brief  Функция генерит таблицу,хранящая начала и концы каждой строки
\param  [in]      firstArray     Первый массив
\param  [in]      nDividers      Количество разделителей в первом массиве
\param  [in,out]  outTable       Указатель на таблицу соответствий
\param  [in,out]  outArraySize   Размер таблицы
\note   Память под таблицу выделять не нужно. Всю память функция выделяет автоматически.
Требуется только указать, где будет храниться указатель на таблицу, именно это значение и
будет записано в outTable.
\return Код ошибки или ноль в случае успеха.
*/
static int generateBijection(const C_string* firstArray, const int nDividers, Link** outTable, int* outArraySize)
{
    Assert_c(firstArray != NULL);
    Assert_c(outTable != NULL);
    if (!firstArray || !outTable)
        return SS_ERROR_NULLPTR;
    Assert_c(*firstArray != NULL);
    if (!*firstArray)
        return SS_ERROR_NULLPTR;

    Link* table = (Link*)calloc(nDividers, sizeof(Link));;
    Assert_c(table != NULL);
    if (!table)
        return SS_ERROR_NULLPTR;
    *outTable = table;

    int tableSize = 0;

    table->start = 0;
    table->start = 0;
    table->end = 0;
    C_string ptr = *firstArray;
    for (int i = 0; i < nDividers; i++)
    {
        table->start = ptr;

        ptr = wcschr(ptr, L'\0');
        Assert_c(ptr != NULL);
        if (!ptr)
            return SS_ERROR_NULLPTR;

        table->end = ptr- 1;

        ptr++;
        if (table->end - table->start > 1)
        {
            table++;
            tableSize++;
        }
    }
    *outTable = (Link*)realloc(*outTable, sizeof(Link)*tableSize);
    *outArraySize = tableSize;
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
2. Строится таблица соответствий между подстроками в STR1 и STR2.
3. Производится сортировка данной таблицы.
4. Результат записываем в выходной файл.
*/
void sortFromFile(const char* inFilename, const char* outFilename, Direction dir)
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
            free(data-1);
        return;
    }


    //Построение таблицы индексов
    Link* table = NULL;
    int tableSize = 0;
    errorCode = generateBijection(&data, nDividers, &table,&tableSize);
    if (errorCode)
    {
        fclose(outFile);
        free(data-1);
        if (table)
            free(table);
        return;
    }

    //сортировка и вывод
    qsort(table, tableSize, sizeof(Link), dir == Direct ? cmpTableDirect : cmpTableInverse);

    for (int i = 0; i < tableSize; i++)
        fprintf(outFile, "%ls\n", table[i].start);

    fclose(outFile);
    free(data-1);
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
static int getPtr(RangedPtr* rptr, int inc = 1)
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
static inline void findRanges(Link** ptrTable, const int arrSize, RangedPtr* range)
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

    short current_ending = table[ptr[0]].ending;

    while (ptr[1] < arrSize &&  current_ending == table[ptr[1]].ending)
        ptr[1] ++;
    ptr[1] --;

    while (0 <= ptr[0] && current_ending == table[ptr[0]].ending)
        ptr[0] --;
    ptr[0] ++;

}



/**
\brief  Функция проверяет, есть ли в строке буквы
\param  [in]  str  Проверяемая строка
\return Возвращает true, если в строке есть буквы.
*/
static bool isThereAlpha(C_string str)
{
    while (*str)
    {
        if (std::iswalpha(*str))
            return true;
        str++;
    }
    return false;
}

/**
\brief  Функция проверяет, является ли символ гласной буквой
\param  [in]  chr  Проверяемый символ
\return Возвращает true, если буква гласная.
\note   Функция не чувствует регистра.
*/
static inline bool isVowel(wchar_t chr)
{
    C_string vowelStr = L"уеыаоэяиюё";
    return (bool)wcschr(vowelStr, tolower(chr));
}

/**
\brief  Функция подсчитывает количество гласных в строке
\param  [in]  str   Входящая строк
\return Количество гласных букв
*/
static int getNumberVowel(C_string str)
{
    Assert_c(str != NULL);
    if (!str)
        return 0;
    int result = 0;
    while (*str)
    {
        if (isVowel(*str))
            result++;
        str++;
    }
    return result;
}


/**
\brief Функция реализует поиск рифмующегося слова, основываясь на предыдущем
\param [in]      ptrTable  Указатель на таблицу
\param [in,out]  range     Циклический указатель, в который будет записан реузльтат поиска
*/
static int getNextRytm(Link** ptrTable, RangedPtr* range)
{
    const Link* table = *ptrTable;

    int attempt = PG_NUMBER_ATTEMPT;
    bool isVowelFlag = table[range->ptr].isPenultimateVowel;
    int nVowelFlag = table[range->ptr].nVowel;
    do
    {
        getPtr(range);
        attempt--;
    } while (isVowelFlag != table[range->ptr].isPenultimateVowel && abs(nVowelFlag - table[range->ptr].nVowel) != 1 && attempt);

    if (!attempt)
    {
        printf("We have not a lot of attempt!!\nGet:%ls\n", table[range->ptr].start);
    }
    return range->ptr;
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
static void stanzaGenerator(Link** ptrTable, const int arrSize, const char* inSyllableStr, const char* outFilename, const char* outSyllableStr, const int nStanzas = 1)
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
            int attept = PG_NUMBER_ATTEMPT;
            do
            {
                syllableInterval[j].start = randInt(ryhmInterval[j].start, ryhmInterval[j].end);
                syllableInterval[j].end = syllableInterval[j].start;
                findRanges(&table, arrSize, &syllableInterval[j]);
                attept--;
            } while (syllableInterval[j].start == syllableInterval[j].end && attept);
            syllableInterval[j].ptr = randInt(syllableInterval[j].start, syllableInterval[j].end);
        }


        for (int i = 0; i < outSyllableLength; i++)
        {
            int index = getNextRytm(&table, &syllableInterval[outSyllable[i] - 'a']);
            fprintf(poemFile, "%ls\n", table[index].start);
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
void poermGenerator(const char* inFilename, const char* inSyllableStr, const char* outFilename, const char* outSyllableStr, const int nStanzas)
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
            free(data - 1);
        return;
    }


    //Построение таблицы индексов
    Link* table = NULL;
    int tableSize = 0;
    errorCode = generateBijection(&data, nDividers, &table, &tableSize);
    if (errorCode)
    {
        fclose(outFile);
        free(data - 1);
        if (table)
            free(table);
        return;
    }


    //инициализация входных слоговых конструкций
    {
        int index = 0;
        int inSyllableLength = strlen(inSyllableStr);
        for (int i = 0; i < tableSize; i++)
        {
            if (!isThereAlpha(table[i].start))
            {
                table[i].rythmType = 0;
                table[i].nVowel = 0;
                continue;
            }


            wchar_t ending[3] = {};
            for (int nLetters = 0, j = 0; nLetters < 3 && table[i].end[-j]; j++)
            {
                if (std::iswalpha(table[i].end[-j]))
                    ending[nLetters++] = table[i].end[-j];
            }
            table[i].isPenultimateVowel = isVowel(ending[2]);
            table[i].ending = tolower(ending[0])-L'а' + 33 * (tolower(ending[1])- L'а');

            table[i].rythmType = tolower(inSyllableStr[index]) - 'a' + 1;
            index++;
            index %= inSyllableLength;
            table[i].nVowel = getNumberVowel(table[i].start);
        }
    }

    //составление словаря рифм
    qsort(table, tableSize, sizeof(Link), cmpSyllable);

    //генерируем бред в файл
    stanzaGenerator(&table, tableSize, inSyllableStr, outFilename, outSyllableStr, nStanzas);

    free(data-1);
    free(table);
    return;
}

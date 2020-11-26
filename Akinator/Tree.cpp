#include "Tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <stack>

#include <ctype.h>



const int STANDART_ERROR_CODE = -1;

const char* DOT_LEAF_STYLE = "[shape = \"ellipse\", fillcolor = \"lightgreen\"]";
const char* DOT_NODE_STYLE = "[color = \"black\", fontsize = 24, shape = \"box\", style = \"filled, rounded\", fillcolor = \"lightgray\"]";
const char* DOT_EDGE_STYLE = "[color = \"black\", fontsize = 24]";

typedef char* C_string;
typedef FILE* Stream;


/**
\brief  Функция полностью сичтывает файл
\param  [in]      filename  Имя считываемого файла
\param  [in,out]  outString Указатель на считанную строку
\param  [in]      readBytesPtr  Указатель на unsigned, в котором будет храниться количество считанных байтов
\return В случае успеха возвращается количество прочитанных байт.
Если произошла ошибка, то возвращается константа -1.
*/
int readFullFile(const char* filename, char** outString)
{
    Assert_c(filename);
    Assert_c(outString);
    if (!filename || !outString)
        return STANDART_ERROR_CODE;

    FILE* inputFile = fopen(filename, "rb");
    Assert_c(inputFile);
    if (!inputFile)
        return STANDART_ERROR_CODE;
    if (ferror(inputFile))
        return STANDART_ERROR_CODE;

    fseek(inputFile, 0, SEEK_END);
    long fsize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    char* string = (char*)calloc(fsize + 2, sizeof(char));
    Assert_c(string);
    if (!string)
        return STANDART_ERROR_CODE;

    unsigned nReadBytes = fread(string, sizeof(char), fsize, inputFile);
    fclose(inputFile);
    string[fsize] = 0;

    *outString = string;

    return nReadBytes;
}


/**
\brief  Функция удаляет из строки все спец символы, кроме тех, которые указаны в строке dontDelChar
\param  [in,out]      ptrStr  Указатель на строку в которой будем производить удаление
\param  [in,out]      dontDelChar Строка, задающая набор символов, которые не следует удалять
\return В случае успеха возвращается новая длина строки.
Если произошла ошибка, то возвращается константа ASM_ERROR_CODE.
\note   В новой строке будут только числа,буквы и символы из строки dontDelChar
        Набор символов, заключенный между кавычками "..." будет копироваться полностью
*/
int removeExtraChar(char** ptrStr, const char* dontDelChar)
{
    Assert_c(ptrStr);
    Assert_c(dontDelChar);
    if (!ptrStr || !dontDelChar)
        return STANDART_ERROR_CODE;
    char* str = *ptrStr;
    Assert_c(str);
    if (!str)
        return STANDART_ERROR_CODE;

    unsigned i = 0;
    unsigned j = 0;

    bool isStrChar = 0;
#define isValid(x) (isalpha(x) || isdigit(x) || strchr(dontDelChar , x) )
    while (str[j])
    {
        if (str[j] == '\"')
        {
            isStrChar ^= 1;
            j++;
            continue;
        }
        if (isValid(str[j]) || isStrChar)
        {
            str[i] = str[j];
            i++;
        }
        j++;
    }
    str[i++] = 0;
    i++;
#undef isValid
    str = (char*)realloc(str, i);
    Assert_c(str);
    if (!str)
        return STANDART_ERROR_CODE;

    *ptrStr = str;

    return i;
}





static void readInteration(Node* root, C_string* const ptrStr)
{
    Assert_c(root);
    Assert_c(*ptrStr);
    if (!root || !*ptrStr)
        return;

    char* ptr = *ptrStr + 1;
    while (!strchr("()", *ptr))
        ptr++;


    int len = ptr - *ptrStr - 1;
    root->data = (C_string)calloc(len + 1, sizeof(char));
    memcpy(root->data, *ptrStr + 1, len);
    root->data[len] = 0;

    if (*ptr == '(')
    {
        root->l = (Node*)calloc(1, sizeof(Node));
        root->r = (Node*)calloc(1, sizeof(Node));
        readInteration(root->l, &ptr);
        readInteration(root->r, &ptr);
    }
    else
    {
        root->l = NULL;
        root->r = NULL;
    }
    ptr++;
    *ptrStr = ptr;
}

static void delInteration(Node* root)
{
    if (!root)
        return;
    delInteration(root->l);
    delInteration(root->r);
    if (root->data)
        free(root->data);
    free(root);
}

static void printInteration(Node* root, int level, Stream stream)
{
    Assert_c(stream);
    Assert_c(level >= 0);
    if (!stream || ferror(stream) || level < 0)
        return;
    if (!root)
        return;


    for (int i = 0; i < level; i++)
        fprintf(stream, "  ");
    fprintf(stream, "(\n");

    for (int i = 0; i < level; i++)
        fprintf(stream, "  ");
    fprintf(stream, "\"%s\"\n", root->data);


    printInteration(root->l, level + 1, stream);
    printInteration(root->r, level + 1, stream);


    for (int i = 0; i < level; i++)
        fprintf(stream, "  ");
    fprintf(stream, ")\n");

}

static void drawTreeInteration(Node* root, FILE* file)
{
    Assert_c(file);
    Assert_c(root);
    if (!file || !root || ferror(file))
        return;

    bool isLeaf = !root->l || !root->r;

    if (isLeaf)
    {
        fprintf(file,
            "\"%s\" %s \n", root->data, DOT_LEAF_STYLE
        );
    }
    else
    {
        drawTreeInteration(root->l, file);
        drawTreeInteration(root->r, file);
        fprintf(file, " \"%s\" \n", root->data);

        fprintf(file, " \"%s\" -> \"%s\" [label = \"No\", fontcolor = \"red\" ] \n", root->data, root->r->data);
        fprintf(file, " \"%s\" -> \"%s\" [label = \"Yes\", fontcolor = \"green\" ] \n", root->data, root->l->data);
    }
}



void Tree::init(const C_string filename)
{
    Assert_c(!root);
    if (root)
        return;

    Assert_c(filename);
    if (!filename)
        return;

    root = (Node*)calloc(1, sizeof(Node));

    char* buffer = NULL;
    int errorCode = 0;
    errorCode = readFullFile(filename, &buffer);
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(root);
        free(buffer);
        root = NULL;
        return;
    }

    errorCode = removeExtraChar(&buffer, "()");
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(root);
        free(buffer);
        root = NULL;
        return;
    }

    C_string ptrStr = buffer;
    readInteration(root, &ptrStr);
    free(buffer);
}

void Tree::destr()
{
    delInteration(root);
}

void Tree::print(Stream stream)
{
    Assert_c(stream);
    if (!stream || ferror(stream))
        return;

    printInteration(root, 0, stream);
}

void Tree::draw(const char* filename)
{
    Assert_c(filename);
    if (!filename)
        return;
    FILE* file = fopen(filename, "w");
    Assert_c(file);
    if (!file)
        return;

    fprintf(file,
        "digraph{\n"
        "node %s\n"
        "edge %s\n"
        , DOT_NODE_STYLE, DOT_EDGE_STYLE
    );

    drawTreeInteration(root, file);


    fprintf(file,
        "}"
    );

    fclose(file);

    char commandStr[256];
    sprintf(commandStr, "dot -Tpng %s -o %s.png", filename, filename);
    system(commandStr);
}



static bool getDefinitionInteration(const Node* root, const C_string word, std::stack<bool>* path)
{
    Assert_c(word);
    if (!word || !root)
        return 0;

    if (!strcmp(word, root->data))
        return 1;

    ui8 res = 0;

    res |= getDefinitionInteration(root->l, word, path) << 0;
    res |= getDefinitionInteration(root->r, word, path) << 1;

    if (res & (1 << 0))
        path->push(0);
    if (res & (1 << 1))
        path->push(1);

    return res;

}

void Tree::getDefinition(const C_string word)
{
    static const C_string ANSW_STR[2] = { "yes","no" };
    bool index = 0;
    std::stack<bool> way;
    Node* node = root;
    getDefinitionInteration(node, word, &way);

    if (way.empty())
    {
        printf("We can't find word:\"%s\" in our base.\n", word);
        return;
    }
    printf("Definition of the word \"%s\": ", word);
    while (!way.empty())
    {
        index = way.top();
        printf("%s (%s) ---> ", node->data, ANSW_STR[index]);

        //Важно, что в структуре Node* l и Node* r располагаются последовательно
        node = (&node->l)[index];
        way.pop();
    }
    printf("%s\n", node->data);
}
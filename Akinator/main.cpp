#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

#include <ctype.h>

#ifdef __GNUG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#define Assert_c(expr) if(!(expr)) printf("Expression %s is false.\nIn file: %s\nfunction: %s\nline: %d\n", #expr, __FILE__, __FUNCSIG__, __LINE__);

const int STANDART_ERROR_CODE = -1;

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
    assert(filename);
    assert(outString);
    if (!filename || !outString)
        return STANDART_ERROR_CODE;

    FILE* inputFile = fopen(filename, "rb");
    assert(inputFile);
    if (!inputFile)
        return STANDART_ERROR_CODE;
    if (ferror(inputFile))
        return STANDART_ERROR_CODE;

    fseek(inputFile, 0, SEEK_END);
    long fsize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    char* string = (char*)calloc(fsize + 2, sizeof(char));
    assert(string);
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



struct Node
{
    char* data;
    Node* l;
    Node* r;
};

static void readInteration(Node* root,C_string* const ptrStr)
{
    if (!*ptrStr)
        return;
    char* ptr = *ptrStr+1;
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
    free(root);
}

static void printInteration(Node* root, int level, Stream stream)
{
    Assert_c(stream);
    if (!stream)
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


const char* DOT_LEAF_STYLE = "[shape = \"ellipse\", fillcolor = \"lightgreen\"]";
const char* DOT_NODE_STYLE = "node[color = \"black\", fontsize = 24, shape = \"box\", style = \"filled, rounded\", fillcolor = \"lightgray\"]";
const char* DOT_EDGE_STYLE = "edge[color = \"black\", fontsize = 24]";

static void drawTreeInteration(Node* root, FILE* file)
{
    Assert_c(file);
    Assert_c(root);
    if (!file || !root)
        return;

    bool isLeaf = !root->l;

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

struct Tree
{
    Node* root = NULL;
    void init(const C_string filename);
    void destr();
    void print(Stream stream = stdout);
    void draw(const char* filename = "tree.dot");
};

void Tree::init(const C_string filename)
{
    Assert_c(!root);
    if (root)
        return;

    root = (Node*)calloc(1, sizeof(Node));

    char* buffer = NULL;
    int errorCode = 0;
    errorCode = readFullFile(filename, &buffer);
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(root);
        root = NULL;
        return;
    }

    errorCode = removeExtraChar(&buffer, "()");
    if (errorCode == STANDART_ERROR_CODE)
    {
        free(root);
        root = NULL;
        return;
    }

    C_string ptrStr = buffer;
    readInteration(root, &ptrStr);
}

void Tree::destr()
{
    delInteration(root);
}

void Tree::print(Stream stream)
{
    printInteration(root, 0, stream);
}

void Tree::draw(const char* filename)
{
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

    drawTreeInteration(root,file);
        

    fprintf(file,
        "}"
    );

    fclose(file);
    system("dot -Tpng tree.dot -o tree.png");
}


void runAkinator()
{
    Tree base;
    base.init("base.txt");
    
    /*
    l --- y answ
    r --- n answ
    */

    char str[128] = {};
    int len = 0;
    bool isValidAnswer = 0;
    bool isLeaf = 0;
    Node* root = base.root;

    do
    {
        isLeaf = !root->l;
        printf("%s\n(y/n):", root->data);
        do
        {
            isValidAnswer = 1;
            scanf("%[^\n]%*c", str);
            switch (str[0])
            {
            case 'y':
                if (isLeaf)
                    printf("Yay!\n");
                else
                    root = root->l;
                break;
            case 'n':
                if (isLeaf)
                {
                    root->l = (Node*)calloc(1, sizeof(Node));
                    root->r = (Node*)calloc(1, sizeof(Node));

                    root->r->data = root->data;

                    printf(":'(\n");
                    printf("Enter your answer:");

                    scanf("%[^\n]%*c", str);
                    len = strlen(str);
                    root->l->data = (C_string)calloc(len + 1, sizeof(char));
                    memcpy(root->l->data, str, len);
                    root->l->data[len] = 0;


                    printf("Can you explain what's the difference?\n");
                    printf("Difference (no answer given old answer, yes give your word):");

                    scanf("%[^\n]%*c", str);
                    len = strlen(str);
                    root->data = (C_string)calloc(len + 1, sizeof(char));
                    memcpy(root->data, str, len);
                    root->data[len] = 0;
                }
                else
                    root = root->r;
                break;
            default:
                isValidAnswer = 0;
                printf("Invalid answer, try again.");
                break;
            }
        } while (!isValidAnswer);
    } while (!isLeaf);

    Stream outStream = fopen("base.txt", "w");
    base.print(outStream);
    base.draw();
    base.destr();
    if(outStream)
        fclose(outStream);
}



int main()
{
    runAkinator();
    system("pause");
    return 0;
}
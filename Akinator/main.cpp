#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "Tree.h"
#include <stack>

/**
\brief  Функция добавления ветвления в дерево акинатора
\note   Данная функция просто копирует указатели, по которым лежат строки
        соответственно, память под эти строки должна выделяться отдельно.
*/
void addWordInTree(Node* node,C_string question, C_string ansYes, C_string ansNo)
{
    Assert_c(node);
    Assert_c(question);
    Assert_c(ansYes);
    Assert_c(ansNo);
    if (!node || !question || !ansYes || !ansNo)
        return;

    node->l = (Node*)calloc(1, sizeof(Node));
    node->r = (Node*)calloc(1, sizeof(Node));
    
    Assert_c(node->l);
    Assert_c(node->r);
    if (!node->l || !node->r)
    {
        Assert_c(!"We can't acclocate memeory for new node.");
        return;
    }

    node->data = question;
    node->l->data = ansYes;
    node->r->data = ansNo;
}

/**
\brief  Функция запускает диалог с пользователем для добавления нового слова в базу акинатора
\param  [in]  node   указатель на узел, который будет изменяться
*/
void updateAkinnatorBase(Node* node)
{
    char str[128] = {};
    int len = 0;
    C_string strQuestion = NULL;
    C_string strYesAnswer = NULL;
    C_string strNoAnswer = NULL;

    printf(":'(\n");
    printf("Enter your answer:");

    ///очень хочется использовать getLine
    scanf("%[^\n]%*c", str);
    len = strlen(str);
    strYesAnswer = (C_string)calloc(len + 1, sizeof(char));
    memcpy(strYesAnswer, str, len);
    strYesAnswer[len] = 0;

    strNoAnswer = node->data;

    printf(
        "Can you explain what's the difference?\n"
        "Difference (no answer given old answer, yes give your word):"
    );

    scanf("%[^\n]%*c", str);
    len = strlen(str);
    strQuestion = (C_string)calloc(len + 1, sizeof(char));
    memcpy(strQuestion, str, len);
    strQuestion[len] = 0;

    addWordInTree(node, strQuestion, strYesAnswer, strNoAnswer);
}

/**
\brief  Дотошная функция, которая простит ~~~ментора~~~ пользователя ~~~~проверить акинатор~~~~ ответить y или n
\return Если пользователь таки ввел y, то возвращается true, в противном случае возвращается false
*/
bool askUser()
{
    bool isCorrectInput = 0;
    ui8 c = 0;
    bool pingpong = 0;
    do
    {
        printf("(y/n):");
        while ((c = getc(stdin)) == '\n') { ;; }
        while (getc(stdin) != '\n') { ;; }

        isCorrectInput = strchr("yn", c);
        if (!isCorrectInput)
            printf("Invalid input, try again.\n");
    } while (!isCorrectInput);

    return c == 'y' ? 1 : 0;
}

/**
\brief  Запускает акинатора, используя в качестве базы переданное дерево
\param  [in]  base   указатель на дерево, содержащее базу слов игры
*/
void runAkinator(Tree* base)
{
    bool isLeaf = 0;
    Node* root = base->root;
    do
    {
        isLeaf = !root->l || !root->r;
        printf("%s\n", root->data);

        if (askUser())
            if (isLeaf)
                printf("Yay!\n");
            else
                root = root->l;
        else
            if (isLeaf)
                updateAkinnatorBase(root);
            else
                root = root->r;
    } while (!isLeaf);

}


int main()
{
    Tree base;
    base.init("base.txt");
    printf("Do you want to play akinator?\n");

    if (askUser())
    {
        runAkinator(&base);
        Stream outStream = fopen("base.txt", "w");
        base.print(outStream);
        if (outStream)
            fclose(outStream);
    }
    
    base.draw();

    char buffer[64];
    printf("Do you want to find your word in our base?\n");
    while (askUser())
    {
        printf("Enter word:");
        scanf("%[^\n]*c", buffer);
        base.getDefinition(buffer);
        printf("Do you want to find your word in our base one more time?\n");
    }
    base.destr();

    system("pause");
    return 0;
}
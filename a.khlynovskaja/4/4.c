#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 256

// один узел списка
struct Node {
    char *text;
    struct Node *next;
};

int main(void)
{
    char buff[MAX_LEN];
    struct Node *head = NULL;
    struct Node *tail = NULL;

    while (1) 
    {
        printf("Введите строку: ");

        if (fgets(buff, MAX_LEN, stdin) == NULL)
            break;

        if (buff[0] == '.')
            break;

        size_t len = strlen(buff);

        if (len > 0 && buff[len - 1] == '\n') 
        {
            buff[len - 1] = '\0';
            len--;
        }

        char *text = (char *)malloc((len + 1) * sizeof(char));

        if (text == NULL) 
        {
            printf("Ошибка выделения памяти\n");
            return 1;
        }

        strcpy(text, buff);

        struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));

        if (new_node == NULL) 
        {
            free(text);
            printf("Ошибка выделения памяти\n");
            return 1;
        }

        new_node->text = text;
        new_node->next = NULL;

        if (head == NULL) 
        {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    printf("\nСписок строк:\n");

    struct Node *val = head;

    while (val != NULL) 
    {
        printf("%s\n", val->text);
        val = val->next;
    }

    val = head;

    while (val != NULL) 
    {
        struct Node *next = val->next;

        free(val->text);
        free(val);

        val = next;
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUF_SIZE 1

struct line_info {
    off_t offset;
    off_t length;
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("Ошибка открытия файла");
        return 1;
    }

    // таблица строк
    struct line_info *table = NULL;
    size_t count = 0;
    size_t capacity = 0;

    off_t line_start = 0;
    char buf[BUF_SIZE];
    ssize_t n;

    // построение таблицы отступов и длин строк
    while ((n = read(fd, buf, BUF_SIZE)) > 0)
    {
        // получение текущей позиции в файле
        off_t current_pos = lseek(fd, 0L, SEEK_CUR);

        if (buf[0] == '\n')
        {
            off_t length = current_pos - line_start;

            // расширение памяти под таблицу
            if (count == capacity)
            {
                capacity = capacity ? capacity * 2 : 16;
                struct line_info *tmp = (struct line_info *)realloc(table, capacity * sizeof(*table));
                if (!tmp)
                {
                    perror("Ошибка realloc");
                    close(fd);
                    free(table);
                    return 1;
                }
                table = tmp;
            }

            table[count].offset = line_start;
            table[count].length = length;
            count++;

            // след строка начинается сразу за текущей
            line_start = current_pos;
        }
    }

    if (n < 0)
    {
        perror("Ошибка чтения файла");
        close(fd);
        free(table);
        return 1;
    }

    // если файл не заканчивался отступом, то записываем последний остаток текста
    off_t file_end = lseek(fd, 0L, SEEK_CUR);
    if (file_end > line_start)
    {
        if (count == capacity)
        {
            capacity = capacity ? capacity * 2 : 16;
            struct line_info *tmp = (struct line_info *)realloc(table, capacity * sizeof(*table));
            if (!tmp)
            {
                perror("Ошибка realloc");
                close(fd);
                free(table);
                return 1;
            }
            table = tmp;
        }
        table[count].offset = line_start;
        table[count].length = file_end - line_start;
        count++;
    }

    printf("Таблица строк файла (всего строк: %zu):\n", count);
    for (size_t i = 0; i < count; i++)
    {
        printf("Строка %2zu: offset = %lld, length = %lld\n",
               i + 1, (long long)table[i].offset, (long long)table[i].length);
    }

    // ззапрос номеров строк у пользователя
    long num;
    while (1)
    {
        printf("\nВведите номер строки (0 для выхода): ");
        if (scanf("%ld", &num) != 1)
        {
            printf("Ошибка, введите целое число\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (num == 0) 
            break;

        if (num < 1 || (size_t)num > count)
        {
            printf("Строка с таким номером не существует. Допустимо: 1..%zu\n", count);
            continue;
        }

        size_t idx = (size_t)num - 1;
        struct line_info li = table[idx];

        // позиц на начало нужной строки
        if (lseek(fd, li.offset, SEEK_SET) == (off_t)-1)
        {
            perror("Ошибка вызова lseek");
            continue;
        }

        char *out = (char *)malloc(li.length + 1);
        if (!out)
        {
            perror("Ошибка malloc");
            continue;
        }

        ssize_t got = read(fd, out, li.length);
        if (got < 0)
        {
            perror("Ошибка чтения строки");
            free(out);
            continue;
        }

        out[got] = '\0';
        printf("Строка %ld: %s", num, out);

        if (got > 0 && out[got - 1] != '\n')
        {
            printf("\n");
        }

        free(out);
    }

    free(table);
    close(fd);
    return 0;
}

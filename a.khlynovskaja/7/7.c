#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_LINES 1000

typedef struct {
    off_t offset;
    off_t length;
} Line;

int fd;
char *file_data;
off_t file_size;

// Обработчик Ctrl+C
void handle_sigint(int sig)
{
    (void)sig;
    printf("\nНажато Ctrl+C. Программа продолжает работу.\n");
}

// обработчик таймаута (5 секунд вышло)
void handle_sigalrm(int sig)
{
    (void)sig;
    printf("\nВремя вышло (5 секунд)!\n");
    printf("Содержимое файла:\n");
    printf("%.*s\n", (int)file_size, file_data);

    munmap(file_data, file_size);
    close(fd);
    exit(0);
}

int main(int argc, char *argv[])
{
    Line lines[MAX_LINES];
    int cnt = 0;
    off_t line_start = 0;

    // регистрация сигналов
    signal(SIGINT, handle_sigint);
    signal(SIGALRM, handle_sigalrm);

    if (argc != 2)
    {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("Ошибка open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("Ошибка fstat");
        close(fd);
        return 1;
    }

    file_size = st.st_size;
    if (file_size == 0)
    {
        printf("Файл пуст.\n");
        close(fd);
        return 0;
    }

    file_data = (char *)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED)
    {
        perror("Ошибка mmap");
        close(fd);
        return 1;
    }

    // построение таблицы отступов
    for (off_t i = 0; i < file_size; i++)
    {
        if (file_data[i] == '\n')
        {
            lines[cnt].offset = line_start;
            lines[cnt].length = i - line_start + 1; 

            cnt++;
            line_start = i + 1;

            if (cnt >= MAX_LINES) break;
        }
    }

    // учет последней строки, если она не заканчивается на отступ
    if (line_start < file_size && cnt < MAX_LINES)
    {
        lines[cnt].offset = line_start;
        lines[cnt].length = file_size - line_start;
        cnt++;
    }

    printf("\nТаблица строк:\n");
    printf("Строка\tОтступ\tДлина\n");
    for (int i = 0; i < cnt; i++)
    {
        printf("%d\t%lld\t%lld\n",
            i + 1,
            (long long)lines[i].offset,
            (long long)lines[i].length
        );
    }

    // цикл запроса строк
    while (1)
    {
        long number;
        printf("\nВведите номер строки (0 - выход, у вас 5 секунд): ");
        fflush(stdout);
        alarm(5);

        if (scanf("%ld", &number) != 1)
        {
            printf("Ошибка: введите целое число!\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF); 
            alarm(0); 
            continue;
        }

        alarm(0);

        if (number == 0)
        {
            break;
        }

        if (number < 1 || number > cnt)
        {
            printf("Строки с таким номером не существует. Доступно: 1..%d\n", cnt);
            continue;
        }

        printf("Строка %ld: %.*s", 
               number, 
               (int)lines[number - 1].length, 
               file_data + lines[number - 1].offset);
        
        if (file_data[lines[number - 1].offset + lines[number - 1].length - 1] != '\n')
        {
            printf("\n");
        }
    }

    munmap(file_data, file_size);
    close(fd);
    return 0;
}

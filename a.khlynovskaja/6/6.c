#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define INITIAL_CAPACITY 16
#define BUFFER_SIZE 256

// структура для хранения указателя на начало строки и ее длины
typedef struct {
    off_t offset;
    size_t length;  // длина строки в байтах
} LineInfo;

// глобальная переменная для доступа из обработчика сигнала
int global_fd = -1;

// обработчик сигнала SIGALRM (вызывается через 5 секунд неактивности при первом вводе)
void handle_alarm(int sig) 
{
    (void)sig;
    
    printf("\n\nВремя на первый ввод истекло (5 секунд)!\n");
    printf("=== Распечатка всего содержимого файла ===\n");

    if (global_fd != -1) 
    {
        // возвращаемся в начало файла
        lseek(global_fd, 0, SEEK_SET);

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(global_fd, buffer, sizeof(buffer))) > 0) 
        {
            write(STDOUT_FILENO, buffer, bytes_read);
        }
    }

    printf("\n==========================================\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) 
    {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    global_fd = fd;

    // настраиваем обработчик сигнала SIGALRM
    struct sigaction sa;
    sa.sa_handler = handle_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // не используем SA_RESTART, чтобы прервать блокирующий scanf
    if (sigaction(SIGALRM, &sa, NULL) == -1) 
    {
        perror("Ошибка установки обработчика сигнала");
        close(fd);
        return EXIT_FAILURE;
    }

    // динамический массив для таблицы строк
    size_t capacity = INITIAL_CAPACITY;
    size_t line_count = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (!table) 
    {
        perror("Ошибка выделения памяти");
        close(fd);
        return EXIT_FAILURE;
    }

    table[0].offset = 0;
    table[0].length = 0;

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // считываем файл и строим таблицу отступов и длин
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) 
    {
        for (ssize_t i = 0; i < bytes_read; i++) 
        {
            table[line_count].length++;

            if (buffer[i] == '\n') 
            {
                off_t current_pos = lseek(fd, 0L, SEEK_CUR) - bytes_read + i + 1;
                line_count++;

                if (line_count >= capacity) 
                {
                    capacity *= 2;
                    LineInfo *temp = realloc(table, capacity * sizeof(LineInfo));
                    if (!temp) 
                    {
                        perror("Ошибка перераспределения памяти");
                        free(table);
                        close(fd);
                        return EXIT_FAILURE;
                    }
                    table = temp;
                }

                table[line_count].offset = current_pos;
                table[line_count].length = 0;
            }
        }
    }

    if (bytes_read == -1) 
    {
        perror("Ошибка чтения файла");
        free(table);
        close(fd);
        return EXIT_FAILURE;
    }

    if (table[line_count].length > 0) 
    {
        line_count++;
    }

    // отладочный вывод таблицы
    printf("=== Таблица отступов и длин строк ===\n");
    printf("%-8s | %-12s | %-10s\n", "Строка", "Смещение (B)", "Длина (B)");
    printf("-------------------------------------\n");
    for (size_t i = 0; i < line_count; i++) 
    {
        printf("%-8zu | %-12ld | %-10zu\n", i + 1, (long)table[i].offset, table[i].length);
    }
    printf("=====================================\n\n");

    // цикл запроса (5 секунд только на первый ввод)
    int line_num;
    int first_input = 1; // флаг первого ввода

    // запускаем таймер на 5 секунд строго один раз перед первым запросом
    alarm(5);

    while (1) 
    {
        if (first_input) 
        {
            printf("У вас есть 5 секунд на первый ввод номера строки (1-%zu, 0 для выхода): ", line_count);
        } 
        else 
        {
            printf("Введите номер строки (1-%zu, 0 для выхода): ", line_count);
        }
        fflush(stdout);

        int result = scanf("%d", &line_num);

        // снимаем таймер сразу после первого ввода (успешного или нет)
        if (first_input) 
        {
            alarm(0);
            first_input = 0;
        }

        if (result != 1) 
        {
            printf("Некорректный ввод. Попробуйте еще раз.\n");
            while (getchar() != '\n');
            continue;
        }

        if (line_num == 0) 
        {
            printf("Завершение работы.\n");
            break;
        }

        if (line_num < 1 || (size_t)line_num > line_count) 
        {
            printf("Ошибка: строки с номером %d не существует.\n", line_num);
            continue;
        }

        size_t idx = line_num - 1;
        off_t target_offset = table[idx].offset;
        size_t target_len = table[idx].length;

        if (lseek(fd, target_offset, SEEK_SET) == (off_t)-1) 
        {
            perror("Ошибка позиционирования (lseek)");
            break;
        }

        char *line_buf = malloc(target_len + 1);
        if (!line_buf) 
        {
            perror("Ошибка выделения памяти");
            break;
        }

        ssize_t read_bytes = read(fd, line_buf, target_len);
        if (read_bytes == -1) 
        {
            perror("Ошибка чтения строки");
            free(line_buf);
            break;
        }

        line_buf[read_bytes] = '\0';

        printf("Строка %d: %s", line_num, line_buf);
        if (read_bytes > 0 && line_buf[read_bytes - 1] != '\n') 
        {
            printf("\n");
        }

        free(line_buf);
    }

    free(table);
    close(fd);

    return EXIT_SUCCESS;
}

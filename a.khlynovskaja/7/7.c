#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define INITIAL_CAPACITY 16

// структура для хранения указателя на начало строки и ее длины
typedef struct {
    const char *line_start; // указатель на начало строки в отображенной памяти
    size_t length;          // длина строки в байтах
} LineInfo;

// глобальные переменные для доступа из обработчика сигнала
const char *global_file_map = NULL;
size_t global_file_size = 0;

// обработчик сигнала SIGALRM (вызывается при таймауте 5 секунд)
void handle_alarm(int sig) 
{
    (void)sig;
    
    printf("\n\nВремя на первый ввод истекло (5 секунд)!\n");
    printf("=== Распечатка всего содержимого файла ===\n");

    if (global_file_map != NULL && global_file_size > 0) 
    {
        // выводим весь отображенный файл из памяти
        fwrite(global_file_map, 1, global_file_size, stdout);
        fflush(stdout);
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

    // получаем размер файла с помощью fstat(2)
    struct stat st;
    if (fstat(fd, &st) == -1) 
    {
        perror("Ошибка получения информации о файле (fstat)");
        close(fd);
        return EXIT_FAILURE;
    }

    // обработка пустого файла
    if (st.st_size == 0) 
    {
        printf("Файл пуст.\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    // отображаем файл в память (mmap)
    char *file_map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file_map == MAP_FAILED) 
    {
        perror("Ошибка отображения файла в память (mmap)");
        close(fd);
        return EXIT_FAILURE;
    }

    // дескриптор файла больше не нужен после выполнения mmap
    close(fd);

    // сохраняем глобальные указатели для обработчика сигнала
    global_file_map = file_map;
    global_file_size = st.st_size;

    // настраиваем обработчик сигнала SIGALRM
    struct sigaction sa;
    sa.sa_handler = handle_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // прерываем блокирующий ввод (scanf)
    if (sigaction(SIGALRM, &sa, NULL) == -1) 
    {
        perror("Ошибка установки обработчика сигнала");
        munmap(file_map, st.st_size);
        return EXIT_FAILURE;
    }

    // построение таблицы строк
    size_t capacity = INITIAL_CAPACITY;
    size_t line_count = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (!table) 
    {
        perror("Ошибка выделения памяти");
        munmap(file_map, st.st_size);
        return EXIT_FAILURE;
    }

    // первая строка начинается с самого первого байта в отображенной памяти
    table[0].line_start = file_map;
    table[0].length = 0;

    // обходим отображенную область как обычный массив в памяти
    for (off_t i = 0; i < st.st_size; i++) 
    {
        table[line_count].length++;

        if (file_map[i] == '\n') 
        {
            line_count++;

            if (line_count >= capacity) 
            {
                capacity *= 2;
                LineInfo *temp = realloc(table, capacity * sizeof(LineInfo));
                if (!temp) 
                {
                    perror("Ошибка перераспределения памяти");
                    free(table);
                    munmap(file_map, st.st_size);
                    return EXIT_FAILURE;
                }
                table = temp;
            }

            // следующая строка начинается со следующего за '\n' байта
            table[line_count].line_start = &file_map[i + 1];
            table[line_count].length = 0;
        }
    }

    // если последний символ файла не был '\n', учтем последнюю строку
    if (table[line_count].length > 0) 
    {
        line_count++;
    }

    // отладочный вывод таблицы отступов и длин
    printf("=== Таблица отступов и длин строк (mmap) ===\n");
    printf("%-8s | %-12s | %-10s\n", "Строка", "Смещение (B)", "Длина (B)");
    printf("--------------------------------------------\n");
    for (size_t i = 0; i < line_count; i++) 
    {
        // смещение вычисляется арифметикой указателей (line_start - file_map)
        off_t offset = table[i].line_start - file_map;
        printf("%-8zu | %-12ld | %-10zu\n", i + 1, (long)offset, table[i].length);
    }
    printf("============================================\n\n");

    // цикл запроса (5 секунд только на первый ввод)
    int line_num;
    int first_input = 1; // Флаг первого ввода

    // запускаем таймер на 5 секунд строго ОДИН РАЗ перед началом работы с вводом
    alarm(5);

    while (1) {
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

        // отменяем таймер сразу после первого ввода
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
        const char *start = table[idx].line_start;
        size_t len = table[idx].length;

        // вывод строки через printf(3) без лишних копирований
        printf("Строка %d: %.*s", line_num, (int)len, start);
        if (len > 0 && start[len - 1] != '\n') 
        {
            printf("\n");
        }
    }

    // освобождение ресурсов
    free(table);
    munmap(file_map, st.st_size);

    return EXIT_SUCCESS;
}

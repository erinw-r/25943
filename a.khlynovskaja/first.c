#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <unistd.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/resource.h>
#endif

#define MAX_OPTS 100

typedef struct {
    char opt;
    char *arg;
} OptionCmd;

int main(int argc, char *argv[]) {
    int opt;
    OptionCmd stack[MAX_OPTS];
    int opt_count = 0;

    opterr = 0;

    while ((opt = getopt(argc, argv, "ispuU:cC:dV:")) != -1) {
        if (opt == '?') {
            fprintf(stderr, "Ошибка: Недопустимая опция или пропущен аргумент '-%c'\n", optopt);
            return EXIT_FAILURE;
        }
        
        if (opt_count < MAX_OPTS) {
            stack[opt_count].opt = opt;
            stack[opt_count].arg = optarg ? strdup(optarg) : NULL;
            opt_count++;
        } else {
            fprintf(stderr, "Превышено максимальное количество опций\n");
            break;
        }
    }

    if (argc == 1) {
        printf("Программа запущена без аргументов.\n");
        return EXIT_SUCCESS;
    }

    printf("--- Обработка опций в порядке справа налево ---\n");

    for (int i = opt_count - 1; i >= 0; i--) {
        char current_opt = stack[i].opt;
        char *current_arg = stack[i].arg;

        switch (current_opt) {
            case 'i':
                printf("[-i] Запрос реальных и эффективных UID/GID (Доступно в полной мере на POSIX/Linux)\n");
                break;
            case 's':
                printf("[-s] Запрос: стать лидером группы процессов\n");
                break;
            case 'p':
                printf("[-p] Запрос идентификаторов процесса (PID, PPID, PGID)\n");
                break;
            case 'u':
                printf("[-u] Запрос значения ulimit\n");
                break;
            case 'U':
                {
                    long new_ul = atol(current_arg);
                    if (new_ul <= 0) {
                        fprintf(stderr, "[-U] Ошибка: Неудачное значение для ulimit: %s\n", current_arg);
                    } else {
                        printf("[-U] Запрос на изменение ulimit на %ld\n", new_ul);
                    }
                }
                break;
            case 'c':
                printf("[-c] Запрос размера core-файла\n");
                break;
            case 'C':
                printf("[-C] Изменение размера core-файла на: %s\n", current_arg);
                break;
            case 'd':
                {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("[-d] Текущая директория: %s\n", cwd);
                    } else {
                        perror("[-d] Ошибка getcwd");
                    }
                }
                break;
            case 'v':
                printf("[-v] Распечатка переменных среды окружения\n");
                break;
            case 'V':
                printf("[-V] Внесение/изменение переменной среды: %s\n", current_arg);
                break;
        }

        if (current_arg) {
            free(current_arg);
        }
    }

    return EXIT_SUCCESS;
}

/* 9.c — создание подпроцесса с cat(1) длинного файла */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    if (argc != 2) {
        fprintf(stderr, "Использование: %s <файл>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    switch (pid = fork()) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0:
            execlp("cat", "cat", argv[1], (char *)NULL);
            perror("execlp");
            _exit(EXIT_FAILURE);

        default:
            printf("Родитель: мой pid = %d, pid потомка = %d\n",
                   getpid(), pid);
            printf("Эта строка выводится сразу, cat работает параллельно\n");

            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status))
                printf("Потомок завершился с кодом %d\n",
                       WEXITSTATUS(status));

            printf(">>> Последняя строка родителя (ПОСЛЕ waitpid) <<<\n");
    }

    return EXIT_SUCCESS;
}
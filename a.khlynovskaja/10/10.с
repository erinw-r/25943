/* 10.c — запуск команды в порожденном процессе и печать её кода завершения */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    switch (pid = fork()) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0:                     /* порожденный процесс */
            /* argv[1] — команда, argv[1..argc-1] — её аргументы;
               argv[argc] == NULL по стандарту, это нужно execvp */
            execvp(argv[1], &argv[1]);
            perror("execvp");       /* попадём сюда только при ошибке */
            _exit(127);

        default:                    /* родитель */
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status))
                printf("Команда завершилась, код завершения: %d\n",
                       WEXITSTATUS(status));
            else if (WIFSIGNALED(status))
                printf("Команда завершена сигналом %d\n",
                       WTERMSIG(status));
    }

    return EXIT_SUCCESS;
}
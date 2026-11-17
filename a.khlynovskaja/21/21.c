#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t beep_count = 0;
static volatile sig_atomic_t quit_flag  = 0;

static void sig_int(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "\a", 1);   
    beep_count++;
}

static void sig_quit(int sig)
{
    (void)sig;
    quit_flag = 1;
}

int main(void)
{
    struct sigaction sa;

    sa.sa_handler = sig_int;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = sig_quit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction SIGQUIT");
        exit(EXIT_FAILURE);
    }

    printf("Нажимайте CTRL-C (звук), CTRL-\\ — выход со счётчиком.\n");

    for (;;) {
        pause();                
        if (quit_flag) {
            printf("\nСигнал прозвучал %d раз(а). Завершение.\n",
                   beep_count);
            break;
        }
    }

    return EXIT_SUCCESS;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

int main() {
    time_t t;
    struct tm *utc_time;

    t = time(NULL);
    if (t == (time_t)-1) {
        perror("Ошибка получения времени");
        return EXIT_FAILURE;
    }


    time_t pst_seconds = t - (8 * 60 * 60);
    struct tm *california_time = gmtime(&pst_seconds);

    if (california_time == NULL) {
        fprintf(stderr, "Ошибка расчета времени\n");
        return EXIT_FAILURE;
    }

    printf("Текущая дата и время в Калифорнии (PST, жесткое смещение -8ч от UTC):\n");
    printf("%02d.%02d.%d %02d:%02d:%02d\n",
           california_time->tm_mday,
           california_time->tm_mon + 1,
           california_time->tm_year + 1900,
           california_time->tm_hour,
           california_time->tm_min,
           california_time->tm_sec);

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

static const char *base_name(const char *path)
{
    const char *p = strrchr(path, '/');
    return (p == NULL) ? path : p + 1;
}

static void print_info(const char *path)
{
    struct stat st;
    struct passwd *pw;
    struct group  *gr;
    char mode[11];
    char timebuf[32];
    struct tm *tm;

    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    if (S_ISDIR(st.st_mode))       mode[0] = 'd';
    else if (S_ISREG(st.st_mode))  mode[0] = '-';
    else                           mode[0] = '?';

    mode[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    mode[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    mode[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    mode[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    mode[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    mode[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    mode[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    mode[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    mode[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    mode[10] = '\0';

    pw = getpwuid(st.st_uid);
    gr = getgrgid(st.st_gid);

    tm = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm);

    printf("%s %3lu %-8s %-8s ",
           mode,
           (unsigned long)st.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?");

    if (S_ISREG(st.st_mode))
        printf("%10ld ", (long)st.st_size);
    else
        printf("%10s ", "");

    printf("%s %s\n", timebuf, base_name(path));
}

int main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл> [файл...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (i = 1; i < argc; i++)
        print_info(argv[i]);

    return EXIT_SUCCESS;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define MAXCOL 40         

static struct termios saved;

static void restore_term(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved);
}

static void erase_char(void)
{
    printf("\b \b");
}

int main(void)
{
    struct termios t;
    char buf[MAXCOL];     
    int n = 0;            
    char c;
    char erase_ch, kill_ch;
    const char werase_ch = 0x17;  
    const char eof_ch    = 0x04;   

    if (tcgetattr(STDIN_FILENO, &t) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    saved = t;
    atexit(restore_term);

    erase_ch = t.c_cc[VERASE];
    kill_ch  = t.c_cc[VKILL];

    t.c_lflag &= ~(ECHO | ICANON);
    t.c_cc[VMIN]  = 1;  
    t.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    printf("Редактируйте строку (ERASE/KILL/CTRL-W), CTRL-D в начале — выход.\n");

    for (;;) {
        if (read(STDIN_FILENO, &c, 1) != 1)
            break;

        if (c == eof_ch && n == 0)
            break;                      

        if (c == erase_ch) {          
            if (n > 0) {
                n--;
                erase_char();
            }
        }
        else if (c == kill_ch) {       
            while (n > 0) {
                n--;
                erase_char();
            }
        }
        else if (c == werase_ch) {       
            while (n > 0 && buf[n - 1] == ' ') {
                n--;
                erase_char();
            }
            while (n > 0 && buf[n - 1] != ' ') {
                n--;
                erase_char();
            }
        }
        else if (isprint((unsigned char)c)) {
            if (n == MAXCOL) {          
                if (buf[n - 1] != ' ') {
                    int k = n - 1, wlen, i;
                    while (k > 0 && buf[k - 1] != ' ')
                        k--;
                    wlen = n - k;
                    if (k == 0) {
                        printf("\r\n");
                        n = 0;
                    } else {
                        for (i = 0; i < wlen; i++)
                            erase_char();
                        printf("\r\n");
                        memmove(buf, buf + k, wlen);
                        n = wlen;
                        fwrite(buf, 1, n, stdout);
                    }
                } else {                
                    printf("\r\n");
                    n = 0;
                }
            }
            buf[n++] = c;
            putchar(c);                
        }
        else {
            putchar('\a');              
        }
        fflush(stdout);
    }

    printf("\nЗавершение.\n");
    return EXIT_SUCCESS;
}
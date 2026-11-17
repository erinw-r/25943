all: res

res: 18.c
	gcc -Wall -Wextra -Wpedantic -g 18.c -o res

run: res
	./res 18.c . /tmp Makefile

clean:
	rm -f res
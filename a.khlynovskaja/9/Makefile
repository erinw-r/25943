all: res

res: 9.c
	gcc -Wall -Wextra -Wpedantic -g 9.c -o res

run: res
	./res test.txt

clean:
	rm -f res
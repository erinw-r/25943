all: res

res: 6.c
	gcc -Wall -Wextra -Wpedantic -g 6.c -o res

run: res
	./res test.txt

clean:
	rm -f res

all: res

res: 7.c
	gcc -Wall -Wextra -Wpedantic -g 7.c -o res

run: res
	./res test.txt

clean:
	rm -f res

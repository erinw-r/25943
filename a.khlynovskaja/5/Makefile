all: res

res: 5.c
	gcc -Wall -Wextra -Wpedantic -g 5.c -o res

run: res
	./res test.txt

clean:
	rm -f res

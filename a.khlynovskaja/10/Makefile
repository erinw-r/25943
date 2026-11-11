all: res

res: 10.c
	gcc -Wall -Wextra -Wpedantic -g 10.c -o res

run: res
	./res ls -l

clean:
	rm -f res
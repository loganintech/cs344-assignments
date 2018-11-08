build: smallsh

test: build
	./grading_script.sh smallsh.out

smallsh:
	gcc -ggdb -o smallsh.out src/smallsh.c

clean:
	rm smallsh.out

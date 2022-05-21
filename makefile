debug:
	gcc -Wall -g myls.c -o myls
release:
	gcc -Wall myls.c -o myls
clean:
	rm ./myls

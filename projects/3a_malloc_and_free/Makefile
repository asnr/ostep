run: mem_test
	echo '>>>>>>>>>'
	./mem_test

mem_test: mem_test.c mem.so
	gcc -lmem -L. -o mem_test mem_test.c -Wall -Werror

mem.so: mem.h mem.c
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o

clean:
	rm mem.o libmem.so mem_test

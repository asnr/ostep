.PHONY: all
all : libcrawler.so file_tester web_tester

test_file: file_tester
	./file_tester pagea

file_tester : file_tester.c libcrawler.so
	gcc -g -L. -lcrawler -lpthread file_tester.c -Wall -Werror -o file_tester

web_tester : web_tester.c cs537.c libcrawler.so
	gcc -g -L. -lcrawler -lpthread web_tester.c cs537.c -Wall -Werror -o web_tester

libcrawler.so : crawler.c
	gcc -g -fpic -c page_queue.c -Wall -Werror -o page_queue.o
	gcc -g -fpic -c url_queue.c -Wall -Werror -o url_queue.o
	gcc -g -fpic -c parser_pool.c -Wall -Werror -o parser_pool.o
	gcc -g -fpic -c crawler.c -Wall -Werror -o crawler.o
	gcc -g -shared -o libcrawler.so crawler.o page_queue.o url_queue.o parser_pool.o

.PHONY: clean
clean :
	rm -f file_tester web_tester libcrawler.so *.o *~

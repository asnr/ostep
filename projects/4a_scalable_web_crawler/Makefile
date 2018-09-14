.PHONY: all
all : libcrawler.so file_tester web_tester

test_file: file_tester
	./file_tester pagea

test_no_cycles: file_tester
	./file_tester file_web_no_cycles/page_0

file_tester : file_tester.c libcrawler.so
	gcc -g -L. -lcrawler -lpthread file_tester.c -Wall -Werror -o file_tester

test_address_sanitizer: file_tester_address_sanitizer
	./file_tester_address_sanitizer file_web_no_cycles/page_0

file_tester_address_sanitizer : file_tester.c libcrawler.so
	clang -fsanitize=address -g -L. -lcrawler -lpthread file_tester.c -Wall -Werror -o file_tester_address_sanitizer

web_tester : web_tester.c cs537.c libcrawler.so
	gcc -g -L. -lcrawler -lpthread web_tester.c cs537.c -Wall -Werror -o web_tester

page_queue.o: page_queue.c page_queue.h
	gcc -g -fpic -c page_queue.c -Wall -Werror -o page_queue.o

url_queue.o: url_queue.c url_queue.h
	gcc -g -fpic -c url_queue.c -Wall -Werror -o url_queue.o

job_counter.o: job_counter.c job_counter.h
	gcc -g -fpic -c job_counter.c -Wall -Werror -o job_counter.o

string_set.o: string_set.c string_set.h
	gcc -g -fpic -c string_set.c -Wall -Werror -o string_set.o

parser_pool.o: parser_pool.c parser_pool.h
	gcc -g -fpic -c parser_pool.c -Wall -Werror -o parser_pool.o

downloader_pool.o: downloader_pool.c downloader_pool.h
	gcc -g -fpic -c downloader_pool.c -Wall -Werror -o downloader_pool.o

crawler.o: crawler.c
	gcc -g -fpic -c crawler.c -Wall -Werror -o crawler.o

libcrawler.so : page_queue.o url_queue.o job_counter.o string_set.o parser_pool.o downloader_pool.o crawler.o
	gcc -g -shared -o libcrawler.so \
		crawler.o page_queue.o url_queue.o job_counter.o string_set.o parser_pool.o downloader_pool.o

.PHONY: clean
clean :
	rm -f file_tester web_tester libcrawler.so *.o *~

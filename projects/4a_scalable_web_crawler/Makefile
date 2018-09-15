LIBCRAWLER_OBJS = \
	obj/libcrawler/page_queue.o\
	obj/libcrawler/url_queue.o\
	obj/libcrawler/job_counter.o\
	obj/libcrawler/string_set.o\
	obj/libcrawler/parser_pool.o\
	obj/libcrawler/downloader_pool.o\
	obj/libcrawler/crawler.o

LIBCRAWLER_HEADERS = \
	src/libcrawler/page_queue.h\
	src/libcrawler/url_queue.h\
	src/libcrawler/job_counter.h\
	src/libcrawler/string_set.h\
	src/libcrawler/parser_pool.h\
	src/libcrawler/downloader_pool.h\
	src/libcrawler/crawler.h

.PHONY: all
all : libcrawler.so file_tester web_tester

test_file: bin/file_tester
	bin/file_tester pagea

test_no_cycles: bin/file_tester
	bin/file_tester file_web_no_cycles/page_0

test_address_sanitizer: bin/file_tester_address_sanitizer
	bin/file_tester_address_sanitizer file_web_no_cycles/page_0

bin/file_tester : src/file_tester.c obj/libcrawler/libcrawler.so
	gcc -g -Lobj/libcrawler -lcrawler -lpthread src/file_tester.c -Wall -Werror -o bin/file_tester

bin/file_tester_address_sanitizer : src/file_tester.c obj/libcrawler/libcrawler.so
	clang -fsanitize=address -g -Lobj/libcrawler -lcrawler -lpthread src/file_tester.c -Wall -Werror -o bin/file_tester_address_sanitizer

bin/web_tester : src/web_tester.c src/cs537.c obj/libcrawler/libcrawler.so
	gcc -g -Lobj/libcrawler -lcrawler -lpthread src/web_tester.c src/cs537.c -Wall -Werror -o web_tester

obj/libcrawler/%.o: src/libcrawler/%.c $(LIBCRAWLER_HEADERS)
	gcc -g -fpic -c $< -Wall -Werror -o $@

obj/libcrawler/libcrawler.so: $(LIBCRAWLER_OBJS)
	gcc -g -shared -o obj/libcrawler/libcrawler.so $(LIBCRAWLER_OBJS)

.PHONY: clean
clean :
	rm -f bin/* obj/*.o obj/libcrawler/*

test: mem_internal_test mem_user_test
	echo '>> Internal Test >>>>>>>>>>>>'
	./mem_internal_test
	echo ''
	echo '>> User Test >>>>>>>>>>>>>>>>'
	./mem_user_test

perf_test: mem_perf_test
	./mem_perf_test

perf_measure: mem_perf_test
	./mem_perf_test 10000
	./mem_perf_test 20000
	./mem_perf_test 40000

mem_user_test: mem_user_test.c mem.so
	gcc -lmem -L. -o mem_user_test mem_user_test.c -Wall -Werror

mem_internal_test: mem_internal_test.c mem.so
	gcc -lmem -L. -o mem_internal_test mem_internal_test.c -Wall -Werror

mem_perf_test: mem_perf_test.c mem.so
	gcc -lmem -L. -o mem_perf_test mem_perf_test.c -Wall -Werror

view: mem_view
	echo '>> View >>>>>>>>>>>>>>>>>>>>>'
	./mem_view

mem_view: mem_view.c mem.so
	gcc -lmem -L. -o mem_view mem_view.c -Wall -Werror

mem.so: mem.h mem.c
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o

clean:
	rm mem.o libmem.so mem_user_test

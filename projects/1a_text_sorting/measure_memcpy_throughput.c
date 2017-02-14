/*
The memory bandwidth I expect (on my early 2015 Macbook Pro with 16GB
of DDR3 RAM):

    memory bus width: 64 bits = 8 bytes
    memory clock frequency: 1867 MHz
    DDR multiplier: 2
    dual channel multiplier: 2

    8 * 1867*10^6 * 2 * 2 = 59.744 GB/s

WRITE SPEED?

Thus time to read 1GB is approx. 17ms.

Representative output of this code:

    
*/


#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

static const long NUM_BYTES_TO_COPY = 2000000000;

void exit_if_malloc_failed(void *buf) {
    if (buf == NULL) {
        printf("Error: malloc() failed.\n");
        exit(1);
    }
}

int main(int argc, char const *argv[])
{

    char *read_buf = (char *) malloc(NUM_BYTES_TO_COPY);
    exit_if_malloc_failed(read_buf);

    char *write_buf = (char *) malloc(NUM_BYTES_TO_COPY);
    exit_if_malloc_failed(write_buf);

    // srand(time(NULL));
    // long rand_num = rand() % NUM_BYTES_TO_COPY;

    // Start timer
    struct timeval start_time, end_time;
    int startrc = gettimeofday(&start_time, NULL);
    if (startrc == -1) {
        printf("Error: start gettimeofday() failed.\n");
        exit(1);
    }

    memcpy(write_buf, read_buf, NUM_BYTES_TO_COPY);
    
    // End timer
    int endrc = gettimeofday(&end_time, NULL);
    if (endrc == -1) {
        printf("Error: start gettimeofday() failed.\n");
        exit(1);
    }

    long usecs_elapsed = 1000000 * (end_time.tv_sec - start_time.tv_sec) +
                         (end_time.tv_usec - start_time.tv_usec);

    // printf("Last char in buffer: %d, %c\n", (int) last_char, last_char);

    printf("Microseconds to memcpy %ld bytes (%.1f GB): %ld\n",
           NUM_BYTES_TO_COPY, NUM_BYTES_TO_COPY / (double) 1000000000,
           usecs_elapsed);
    printf("This equals a copy rate of %.2f GB/s\n",
           (NUM_BYTES_TO_COPY / (double) 1000) / (double) usecs_elapsed);
    
    free(write_buf);
    free(read_buf);

    return 0;
}
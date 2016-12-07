#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>

static const int PAGESIZE = 4096;  // enter `pagesize` into the terminal

void print_usage()
{
    fprintf(stderr,
            "Usage: ./tlb numpages numtrials\n  numpages - number of pages to touch\n  numtrials - number of trials to run\n");
}

long strtol_exit_if_fail(const char *str, const char *varname)
{
    char *endptr;
    long value = strtol(str, &endptr, 10);
    if (*str == '\0' || *endptr != '\0') {
        fprintf(stderr,
                "Error: supplied arg \"%s\" is not an integer\n", varname);
        print_usage();
        exit(1);
    }

    return value;
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        print_usage();
        exit(1);
    }

    int numpages = (int) strtol_exit_if_fail(argv[1], "num_pages_to_touch");
    int numtrials = (int) strtol_exit_if_fail(argv[2], "num_trials");

    size_t arrsz = numpages * (PAGESIZE / sizeof(int));
    int *arr = (int *) calloc(arrsz, sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "Error: calloc() failed\n");
        exit(1);
    }

    size_t jump = PAGESIZE / sizeof(int);

    struct timeval start, stop;

    // Warm up TLB any other relevant buffers
    for (size_t idx = 0; idx < arrsz; idx += jump) {
        arr[idx] += 1;
    }

    int rc1 = gettimeofday(&start, NULL);
    if (rc1 == -1) {
        fprintf(stderr, "Error: first gettimeofday() failed\n");
        exit(1);
    }

    for (int trial = 0; trial < numtrials; trial++) {
        for (size_t idx = 0; idx < arrsz; idx += jump) {
            arr[idx] += 1;
        }
    }

    int rc2 = gettimeofday(&stop, NULL);
    if (rc2 == -1) {
        fprintf(stderr, "Error: second gettimeofday() failed\n");
        exit(1);
    }

    long usecs = 1000000 * (stop.tv_sec - start.tv_sec) +
                 (stop.tv_usec - start.tv_usec);
    printf("%ld\n", usecs);

    // Print random entry in array to standard out to prevent compiler
    // optimising away the loop we want to measure.
    int randidx = rand() % arrsz;
    fprintf(stderr, "%d\n", arr[randidx]);

    return 0;
}

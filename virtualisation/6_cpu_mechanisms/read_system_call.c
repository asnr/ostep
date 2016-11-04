#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

static const int NUM_SAMPLES = 100;

void measure_read_times(int reads_per_sample, int usecs[], int num_samples);

void print_array(int arr[], int size, char *filename);


int main(int argc, char const *argv[])
{
    int usecs_100_reads[NUM_SAMPLES];
    int usecs_1000_reads[NUM_SAMPLES];
    int usecs_10000_reads[NUM_SAMPLES];

    measure_read_times(100, usecs_100_reads, NUM_SAMPLES);
    measure_read_times(1000, usecs_1000_reads, NUM_SAMPLES);
    measure_read_times(10000, usecs_10000_reads, NUM_SAMPLES);

    print_array(usecs_100_reads, NUM_SAMPLES, "read_100.out");
    print_array(usecs_1000_reads, NUM_SAMPLES, "read_1000.out");
    print_array(usecs_10000_reads, NUM_SAMPLES, "read_10000.out");

    return 0;
}


int get_diff_in_usecs(struct timeval end, struct timeval start) {
    return 1000000 * (end.tv_sec - start.tv_sec)
           + (end.tv_usec - start.tv_usec);
}


void measure_read_times(int reads_per_sample, int usecs[], int num_samples) {
    for (int smpl_idx = 0; smpl_idx < num_samples; ++smpl_idx) {
        char fake_buf[1];
        struct timeval start;
        struct timeval end;
        gettimeofday(&start, NULL);
        for (int i = 0; i < reads_per_sample; ++i) {
            read(STDIN_FILENO, fake_buf, 0);
        }
        gettimeofday(&end, NULL);

        usecs[smpl_idx] = get_diff_in_usecs(end, start);
    }
}


void print_array(int arr[], int size, char *filename) {
    FILE *fp = fopen(filename, "w");
    for (int idx = 0; idx < size; ++idx) {
        fprintf(fp, "%d\n", arr[idx]);
    }
    fclose(fp);
}

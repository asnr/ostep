
#include <stdio.h>
#include <sys/time.h>

static const int NUM_SAMPLES = 100000;


void print_raw_times(struct timeval times[], int num_samples);

void print_samples_per_usec(struct timeval times[], int num_samples);

int runtime_distribution(struct timeval times[], int num_samples, double dist[]);


int main(int argc, char const *argv[])
{
    struct timeval times[NUM_SAMPLES];

    for (int sample_idx = 0; sample_idx < NUM_SAMPLES; ++sample_idx) {
        gettimeofday(times + sample_idx, NULL);
    }

    // print_raw_times(times, NUM_SAMPLES);
    print_samples_per_usec(times, NUM_SAMPLES);

    return 0;
}


void print_raw_times(struct timeval times[], int num_samples) {

    time_t curr_secs;
    time_t prev_secs;
    suseconds_t curr_usecs;
    suseconds_t prev_usecs;

    printf("Call 0:\t%ld secs, %d usecs\n",
           times[0].tv_sec, times[0].tv_usec);

    for (int sample_idx = 1; sample_idx < num_samples; ++sample_idx)
    {
        prev_secs = times[sample_idx - 1].tv_sec;
        prev_usecs = times[sample_idx - 1].tv_usec;
        curr_secs = times[sample_idx].tv_sec;
        curr_usecs = times[sample_idx].tv_usec;

        if (curr_secs == prev_secs) {
            if (curr_usecs == prev_usecs) {
                printf("Call %d:\t     '' secs,     '' usecs\n", sample_idx);
            } else {
                printf("Call %d:\t     '' secs, %d usecs\n",
                       sample_idx, curr_usecs);
            }
        } else {
            printf("Call %d:\t%ld secs, %d usecs\n",
                   sample_idx, curr_secs, curr_usecs);
        }
    }

}


int are_equal(struct timeval a, struct timeval b) {
    return a.tv_sec == b.tv_sec && a.tv_usec == b.tv_usec ? 1 : 0;
}

// Note if no next microsecond, then will return index == size, i.e.
// just past the end of the array.
int get_idx_next_usec(struct timeval times[], int size, int start_idx) {
    int idx_next_usec;
    for (idx_next_usec = start_idx + 1; idx_next_usec < size; ++idx_next_usec) {
        struct timeval prev = times[idx_next_usec - 1];
        struct timeval curr = times[idx_next_usec];
        if (!are_equal(curr, prev)) break;
    }
    return idx_next_usec;
}

int get_usec_diff(struct timeval later, struct timeval earlier) {
    return 1000000*(later.tv_sec - earlier.tv_sec)
           + later.tv_usec - earlier.tv_usec;
}


void print_samples_per_usec(struct timeval times[], int num_samples) {
    int idx_last_usec = get_idx_next_usec(times, num_samples, 0);
    int idx_next_usec = get_idx_next_usec(times, num_samples, idx_last_usec);
    while (idx_next_usec < num_samples) {
        int usec_diff = get_usec_diff(times[idx_next_usec],
                                      times[idx_last_usec]);
        if (usec_diff > 1) {
            printf("Difference in usecs is %d\n", usec_diff);
        } else {
            printf("%d\n", idx_next_usec - idx_last_usec);            
        }

        idx_last_usec = idx_next_usec;
        idx_next_usec = get_idx_next_usec(times, num_samples, idx_last_usec);
    }
}


void fill_array(double val, double arr[], int start, int size) {
    int end = start + size;
    for (int idx = start; idx < end; ++idx) {
        arr[idx] = val;
    }
}


// Fills the dist array, which must have at least num_samples long.
// Returns the number of entries of dist that were actually filled
// (the first first values of the times array are ignored up to the
// first new microsecond and the values in the last microsecond are
// also ignored).
int runtime_distribution(struct timeval times[], int num_samples, double dist[]) {

    int dist_size = 0;
    int idx_fst_new_usec = get_idx_next_usec(times, num_samples, 0);
    int idx_last_usec = idx_fst_new_usec;
    int idx_next_usec = get_idx_next_usec(times, num_samples, idx_last_usec);

    while (idx_next_usec < num_samples) {
        int usec_diff = get_usec_diff(times[idx_next_usec], times[idx_last_usec]);
        int samples_in_window = idx_next_usec - idx_last_usec;

        if (samples_in_window > 1 && usec_diff > 1) {
            int samples_in_first_usec = samples_in_window - 1;
            fill_array(1 / (double) samples_in_first_usec,
                       dist, dist_size, samples_in_first_usec);
            double usecs_for_last_sample = (double) usec_diff;
            dist[dist_size + samples_in_first_usec] = usecs_for_last_sample;
        } else {
            fill_array(usec_diff / (double) samples_in_window,
                       dist, dist_size, samples_in_window);
        }
        dist_size += samples_in_window;
        
        idx_last_usec = idx_next_usec;
        idx_next_usec = get_idx_next_usec(times, num_samples, idx_last_usec);
    }

    return dist_size;
}

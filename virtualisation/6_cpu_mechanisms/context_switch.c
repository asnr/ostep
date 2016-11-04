#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#include <mach/task_info.h>
#include <pthread.h>

static const int AFFINITY_TAG_ALL_PROCS = 9;

static const int NUM_SAMPLES = 100;

void set_thread_affinity_tag(int affinity_tag);
void print_thread_affinity_tag(boolean_t expect_default);
void measure_context_switch_times(int switch_pairs_per_sample, int usecs[], int num_samples);
void print_array(int arr[], int size, char *filename);


int main(int argc, char const *argv[])
{
    int usecs_10_switch_pairs[NUM_SAMPLES];
    int usecs_100_switch_pairs[NUM_SAMPLES];
    int usecs_10000_switch_pairs[NUM_SAMPLES];

    measure_context_switch_times(10, usecs_10_switch_pairs, NUM_SAMPLES);
    measure_context_switch_times(100, usecs_100_switch_pairs, NUM_SAMPLES);
    measure_context_switch_times(10000, usecs_10000_switch_pairs, NUM_SAMPLES);

    print_array(usecs_10_switch_pairs, NUM_SAMPLES, "switches_10_pairs.noaff.out");
    print_array(usecs_100_switch_pairs, NUM_SAMPLES, "switches_100_pairs.noaff.out");
    print_array(usecs_10000_switch_pairs, NUM_SAMPLES, "switches_10000_pairs.noaff.out");


    print_thread_affinity_tag(TRUE);

    // Note that according to the docs
    //   https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/#//apple_ref/doc/uid/TP40006635-CH1-DontLinkElementID_2
    // forked children (a) share the same affinity tag namespace and
    // (b) inherit the parent's affinity tag.
    // Thus we should not have to set this affinity tag again.
    set_thread_affinity_tag(AFFINITY_TAG_ALL_PROCS);

    print_thread_affinity_tag(FALSE);

    measure_context_switch_times(10, usecs_10_switch_pairs, NUM_SAMPLES);
    measure_context_switch_times(100, usecs_100_switch_pairs, NUM_SAMPLES);
    measure_context_switch_times(10000, usecs_10000_switch_pairs, NUM_SAMPLES);

    print_array(usecs_10_switch_pairs, NUM_SAMPLES, "switches_10_pairs.aff.out");
    print_array(usecs_100_switch_pairs, NUM_SAMPLES, "switches_100_pairs.aff.out");
    print_array(usecs_10000_switch_pairs, NUM_SAMPLES, "switches_10000_pairs.aff.out");

    return 0;
}


int get_thread_affinity_tag(boolean_t *get_default)
{
    thread_affinity_policy_data_t affinity;
    mach_msg_type_number_t count = THREAD_AFFINITY_POLICY_COUNT;

    thread_policy_get(pthread_mach_thread_np(pthread_self()),
                      THREAD_AFFINITY_POLICY,
                      (thread_policy_t) &affinity,
                      &count,
                      get_default);

    return affinity.affinity_tag;
}


void print_thread_affinity_tag(boolean_t expect_default)
{
    boolean_t get_default = expect_default;
    int affinity_tag = get_thread_affinity_tag(&get_default);
    
    if (expect_default != get_default) {
        printf("Suspicious behaviour! Expected default: %s, actually got default: %s\n",
               expect_default ? "TRUE" : "FALSE",
               get_default ? "TRUE" : "FALSE");
    }

    printf("Affinity tag: %d\n", affinity_tag);
}


void set_thread_affinity_tag(int affinity_tag)
{
    thread_affinity_policy_data_t affinity = { .affinity_tag = affinity_tag };

    thread_policy_set(pthread_mach_thread_np(pthread_self()),
                      THREAD_AFFINITY_POLICY,
                      (thread_policy_t) &affinity,
                      THREAD_AFFINITY_POLICY_COUNT);
}


void print_array(int arr[], int size, char *filename)
{
    FILE *fp = fopen(filename, "w");
    for (int idx = 0; idx < size; ++idx) {
        fprintf(fp, "%d\n", arr[idx]);
    }
    fclose(fp);
}


int context_switch_cost(int num_context_switches);

void measure_context_switch_times(int switch_pairs_per_sample, int usecs[], int num_samples)
{
    for (int idx = 0; idx < num_samples; ++idx) {
        usecs[idx] = context_switch_cost(switch_pairs_per_sample);
    }
}


void open_pipe(int fds[])
{
    int piperc = pipe(fds);
    if (piperc == -1) {
        printf("Error: pipe() failed.\n");
        exit(1);
    }
}

void wait_for_second_child(int read_fd)
{
    char buf[2];
    read(read_fd, buf, 1);
}

void release_first_child(int write_fd)
{
    write(write_fd, ">", 1);
}

int diff_in_usecs(struct timeval end, struct timeval start)
{
    return 1000000 * (end.tv_sec - start.tv_sec)
           + (end.tv_usec - start.tv_usec);
}


// write_first is a boolean.
int switch_and_measure(int num_switch_pairs, int write_first,
                       int read_fd, int write_fd) {
    struct timeval start;
    struct timeval end;
    
    gettimeofday(&start, NULL);

    char buf[2];
    for (int pair_idx = 0; pair_idx < num_switch_pairs; ++pair_idx) {
        if (write_first) {
            write(write_fd, ">", 1);
            read(read_fd, buf, 1);
        } else {
            read(read_fd, buf, 1);
            write(write_fd, ">", 1);
        }
    }

    gettimeofday(&end, NULL);

    return diff_in_usecs(end, start);
}

void send_result_to_parent(int result, int write_fd) {
    write(write_fd, &result, sizeof(int));
}

int get_result_from_first_child_and_wait(int read_fd, int pid_1, int pid_2) {
    int result;
    read(read_fd, &result, sizeof(int));
    waitpid(pid_1, NULL, 0);
    waitpid(pid_2, NULL, 0);
    return result;
}

void close_both_ends(int fds[]) {
    close(fds[0]);
    close(fds[1]);
}

int context_switch_cost(int num_switch_pairs) {
    int fds_fst_child_read_snd_write[2];
    int fds_fst_child_write_snd_read[2];
    int fds_return_result[2];
    open_pipe(fds_fst_child_read_snd_write);
    open_pipe(fds_fst_child_write_snd_read);
    open_pipe(fds_return_result);

    int forkrc_1 = fork();
    if (forkrc_1 == -1) {
        printf("Error: first fork() failed.\n");
        exit(1);
    } else if (forkrc_1 == 0) {
        close(fds_fst_child_write_snd_read[0]);
        close(fds_fst_child_read_snd_write[1]);
        close(fds_return_result[0]);

        wait_for_second_child(fds_fst_child_read_snd_write[0]);

        int usecs_taken = switch_and_measure(num_switch_pairs, 1,
                                             fds_fst_child_read_snd_write[0],
                                             fds_fst_child_write_snd_read[1]);

        send_result_to_parent(usecs_taken, fds_return_result[1]);

        exit(0);
    } else {
        int forkrc_2 = fork();
        if (forkrc_2 == -1) {
            printf("Error: second fork() failed.\n");
            exit(1);
        } else if (forkrc_2 == 0) {            
            close(fds_fst_child_write_snd_read[1]);
            close(fds_fst_child_read_snd_write[0]);
            close_both_ends(fds_return_result);

            release_first_child(fds_fst_child_read_snd_write[1]);

            switch_and_measure(num_switch_pairs, 0,
                               fds_fst_child_write_snd_read[0],
                               fds_fst_child_read_snd_write[1]);
            exit(0);
        } else {
            close_both_ends(fds_fst_child_write_snd_read);
            close_both_ends(fds_fst_child_read_snd_write);
            close(fds_return_result[1]);

            int usecs_taken =
                get_result_from_first_child_and_wait(fds_return_result[0],
                                                     forkrc_1, forkrc_2);
            return usecs_taken;
        }
    }
}
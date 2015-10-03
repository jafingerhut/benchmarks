#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


typedef struct time_snapshot_ {
    struct timeval user_time;
    struct timeval sys_time;
    struct timeval wall_clock_time;
} time_snapshot_t;


void get_time_snapshot (time_snapshot_t *time_snapshot)
{
    struct rusage r;
    int ret_val;

    ret_val = getrusage(RUSAGE_SELF, &r);
    if (ret_val != 0) {
        printf("getrusage failed, err %d", errno);
        exit(1);
    }
    memcpy(&(time_snapshot->user_time), &(r.ru_utime), sizeof(struct timeval));
    memcpy(&(time_snapshot->sys_time), &(r.ru_stime), sizeof(struct timeval));
    ret_val = gettimeofday(&(time_snapshot->wall_clock_time), 0);
    if (ret_val != 0) {
        printf("gettimeofday failed, err %d", errno);
        exit(1);
    }
}


unsigned long timeval_diff_usec (struct timeval *start,
                                 struct timeval *end)
{
    unsigned long diff;

    diff = end->tv_sec - start->tv_sec;
    diff *= 1000000;
    diff += end->tv_usec - start->tv_usec;
    return diff;
}


// Calculate (end - start), separately for user, system, and wall
// clock time.
void time_snapshot_diff (unsigned long *out_user_time_usec,
                         unsigned long *out_sys_time_usec,
                         unsigned long *out_wall_clock_time_usec,
                         time_snapshot_t *in_start,
                         time_snapshot_t *in_end)
{
    if (out_user_time_usec != 0) {
        *out_user_time_usec = timeval_diff_usec(&(in_start->user_time),
                                                &(in_end->user_time));
    }
    if (out_sys_time_usec != 0) {
        *out_sys_time_usec = timeval_diff_usec(&(in_start->sys_time),
                                               &(in_end->sys_time));
    }
    if (out_wall_clock_time_usec != 0) {
        *out_wall_clock_time_usec =
            timeval_diff_usec(&(in_start->wall_clock_time),
                              &(in_end->wall_clock_time));
    }
}


int rand_state = 42;

int
glibc_gcc_rand()
{
    rand_state = 1103515245 * rand_state + 12345;
    return (rand_state & 0x7fffffff);
}


void
init_array (int array[], int array_len)
{
    int i;
    for (i = 0; i < array_len; i++) {
        array[i] = glibc_gcc_rand();
    }
}


int
count_lt_threshold (int array[], int array_len, int threshold_val)
{
    int count;
    int i;

    count = 0;
    for (i = 0; i < array_len; i++) {
        if (array[i] < threshold_val) {
            ++count;
        }
    }
    return count;
}


int
compare_ints (const void *a, const void *b)
{
    int x, y;
    x = *((int *) a);
    y = *((int *) b);
    if (x < y) {
        return -1;
    } else if (x > y) {
        return 1;
    }
    return 0;
}


int
main(int argc, char *argv[])
{
    time_snapshot_t snap1, snap2, snap3, snap4;
    unsigned long user_time_usec1, sys_time_usec1, wall_clock_time_usec1;
    unsigned long user_time_usec2, sys_time_usec2, wall_clock_time_usec2;
    unsigned long user_time_usec3, sys_time_usec3, wall_clock_time_usec3;
    int array_len;
    int num_passes, pass;
    int i;
    int threshold_val;
    int is_sorted;
    int count1, count2;
    int *array;
    unsigned nbytes;

    int max_array_len = (1 << 25);

    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        exit(1);
    }
    nbytes = max_array_len * sizeof(int);
    array = (int *) malloc(nbytes);
    if (array == 0) {
        printf("Could not allocate %d bytes for array\n", nbytes);
        exit(1);
    }

    //for (array_len = 512; array_len <= max_array_len; array_len <<= 1) {
    for (array_len = 32768; array_len <= max_array_len; array_len <<= 1) {
        num_passes = (1 << 27) / array_len;
        init_array(array, array_len);

        threshold_val = 1 << 30;
        get_time_snapshot(&snap1);
        count1 = 0;
        for (pass = 0; pass < num_passes; pass++) {
            for (i = 0; i < array_len; i++) {
                if (array[i] < threshold_val) {
                    //++count1;
                    count1 += array[i];
                }
            }
        }
        get_time_snapshot(&snap2);

        qsort(array, array_len, sizeof(int), compare_ints);
        is_sorted = 1;
        for (i = 0; i < array_len - 1; i++) {
            if (array[i] > array[i+1]) {
                is_sorted = 0;
                break;
            }
        }
        
        get_time_snapshot(&snap3);
        count2 = 0;
        for (pass = 0; pass < num_passes; pass++) {
            for (i = 0; i < array_len; i++) {
                if (array[i] < threshold_val) {
                    //++count2;
                    count2 += array[i];
                }
            }
        }
        get_time_snapshot(&snap4);
        
        time_snapshot_diff(&user_time_usec1, &sys_time_usec1,
                           &wall_clock_time_usec1,
                           &snap1, &snap2);
        time_snapshot_diff(&user_time_usec2, &sys_time_usec2,
                           &wall_clock_time_usec2,
                           &snap2, &snap3);
        time_snapshot_diff(&user_time_usec3, &sys_time_usec3,
                           &wall_clock_time_usec3,
                           &snap3, &snap4);
        printf("%d %d %d random %lu %lu %lu sort %lu %lu %lu sorted %lu %lu %lu %d %d sizeof(int)=%lu\n",
               array_len,
               num_passes,
               is_sorted,
               wall_clock_time_usec1,
               user_time_usec1,
               sys_time_usec1,
               wall_clock_time_usec2,
               user_time_usec2,
               sys_time_usec2,
               wall_clock_time_usec3,
               user_time_usec3,
               sys_time_usec3,
               count1, count2, (unsigned long) sizeof(int));
    }
}

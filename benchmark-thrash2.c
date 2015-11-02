#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

typedef unsigned int uint32;


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


void
init_array (int array[], int array_len)
{
    int i;
    for (i = 0; i < array_len; i++) {
        array[i] = i;
    }
}


int
sum_array (int array[], int array_len)
{
    int i;
    int sum = 0;
    for (i = 0; i < array_len; i++) {
        sum += array[i];
    }
    return sum;
}


uint32 lcg_state;


uint32
init_lcg_state(void)
{
    lcg_state = 0;
    return lcg_state;
}


inline uint32
next_lcg_state1 (void)
{
    // Numerical Recipes LCG (Linear Congruential Generator).  Source:
    // https://en.wikipedia.org/wiki/Linear_congruential_generator
    lcg_state = (1664525 * lcg_state) + 1013904223;
    return lcg_state;
}


void
process_array (int array[], int array_len, int num_passes, int stride)
{
    uint32 pass;
    uint32 offset;
    uint32 i;
    uint32 rand_state;
    uint32 rand_num;
    // This is only intended to work if stride and array_len are
    // powers of 2.  Assume they are.
    uint32 num_strides = array_len / stride;
    uint32 mask = num_strides - 1;
    uint32 mask_width = 0;
    uint32 tmp;

    for (tmp = stride; tmp != 1; tmp >>= 1) {
        ++mask_width;
    }
    printf("dbg: array_len=%d stride=%d num_strides=%u mask=%u mask_width=%u\n",
           array_len, stride, num_strides, mask, mask_width);
    rand_state = init_lcg_state();

    for (pass = 0; pass < num_passes; pass++) {
        for (offset = 0; offset < stride; offset++) {
            for (i = 0; i < num_strides; i++) {
                rand_state = next_lcg_state1();
                rand_num = rand_state & mask;
                ++array[(rand_num << mask_width) + offset];
            }
        }
    }
}


int
main(int argc, char *argv[])
{
    time_snapshot_t snap1, snap2;
    unsigned long user_time_usec, sys_time_usec, wall_clock_time_usec;
    int num_passes;
    int stride;
    int array_len;
    int i;
    int sum;
    int *array;
    unsigned nbytes;

    assert (sizeof(uint32) == 4);
    int max_array_len = (1 << 25);

    if (argc != 2) {
        printf("usage: %s <stride>\n", argv[0]);
        exit(1);
    }
    stride = atoi(argv[1]);
    nbytes = max_array_len * sizeof(int);
    array = (int *) malloc(nbytes);
    if (array == 0) {
        printf("Could not allocate %d bytes for array\n", nbytes);
        exit(1);
    }

    for (array_len = 512; array_len <= max_array_len; array_len <<= 1) {
        num_passes = (1 << 27) / array_len;
        for (i = 0; i < array_len; i++) {
            array[i] = i;
        }

        get_time_snapshot(&snap1);
        process_array(array, array_len, num_passes, stride);
        get_time_snapshot(&snap2);

        sum = sum_array(array, array_len);

        time_snapshot_diff(&user_time_usec, &sys_time_usec,
                           &wall_clock_time_usec,
                           &snap1, &snap2);
        printf("%d %d %d %lu %lu %lu %d sizeof(int)=%lu\n",
               array_len, stride, num_passes,
               wall_clock_time_usec,
               user_time_usec,
               sys_time_usec,
               sum, (unsigned long) sizeof(int));
    }
}

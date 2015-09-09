#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>


typedef struct time_snapshot_ {
    struct timeval user_time;
    struct timeval sys_time;
    struct timeval wall_clock_time;
} time_snapshot_t;


int get_time_snapshot (time_snapshot_t *time_snapshot)
{
    struct rusage r;
    int ret_val;

    ret_val = getrusage(RUSAGE_SELF, &r);
    if (ret_val != 0) {
        // ACLQOS_ERR(0, 0, "getrusage failed, err %d", errno);
        return ret_val;
    }
    memcpy(&(time_snapshot->user_time), &(r.ru_utime), sizeof(struct timeval));
    memcpy(&(time_snapshot->sys_time), &(r.ru_stime), sizeof(struct timeval));
    return gettimeofday(&(time_snapshot->wall_clock_time), 0);
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


int
bebusy2 (void)
{
    int sum = 0;
    int i;

    for (i = 0; i < 1000000000; i++) {
        sum = (sum + 7) * 5;
    }
    return sum;
}


int
bebusy8 (void)
{
    int sum = 0;
    int i;

    for (i = 0; i < 1000000000; i++) {
        sum = (sum + 7) * 5;
        sum = (sum + 11) * 9;
        sum = (sum + 13) * 12;
        sum = (sum + 17) * 15;
    }
    return sum;
}


int
bebusy16 (void)
{
    int sum = 0;
    int i;

    for (i = 0; i < 1000000000; i++) {
        sum = (sum + 7) * 5;
        sum = (sum + 11) * 9;
        sum = (sum + 13) * 12;
        sum = (sum + 17) * 15;
        sum = (sum + 19) * 18;
        sum = (sum + 23) * 20;
        sum = (sum + 29) * 25;
        sum = (sum + 31) * 27;
    }
    return sum;
}


int
main (int argc, char *argv[])
{
    time_snapshot_t snap_start, snap_end;
    unsigned long user_time_usec, sys_time_usec, wall_clock_time_usec;
    int sum;

    get_time_snapshot(&snap_start);
    sum = bebusy2();
    get_time_snapshot(&snap_end);
    time_snapshot_diff(&user_time_usec, &sys_time_usec,
                       &wall_clock_time_usec,
                       &snap_start, &snap_end);

    printf("2 %lu %lu %lu %d\n",
           wall_clock_time_usec,
           user_time_usec,
           sys_time_usec,
           sum);

    get_time_snapshot(&snap_start);
    sum = bebusy8();
    get_time_snapshot(&snap_end);
    time_snapshot_diff(&user_time_usec, &sys_time_usec,
                       &wall_clock_time_usec,
                       &snap_start, &snap_end);

    printf("8 %lu %lu %lu %d\n",
           wall_clock_time_usec,
           user_time_usec,
           sys_time_usec,
           sum);

    get_time_snapshot(&snap_start);
    sum = bebusy16();
    get_time_snapshot(&snap_end);
    time_snapshot_diff(&user_time_usec, &sys_time_usec,
                       &wall_clock_time_usec,
                       &snap_start, &snap_end);

    printf("16 %lu %lu %lu %d\n",
           wall_clock_time_usec,
           user_time_usec,
           sys_time_usec,
           sum);
}

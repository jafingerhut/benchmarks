/* From preliminary testing, it appears that at least some LCGs
 * (linear congruental generators) can generate repeating sequences of
 * length 2^b for any desired number of bits b, if you take the b
 * least significant bits of the LCG state.  Write a program to
 * confirm that this is true, for particular choices of LCG. */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0

typedef unsigned int uint32;
typedef unsigned long uint64;

uint32 lcg_state;

uint32
init_lcg_state(void)
{
    lcg_state = 0;
    return lcg_state;
}


// The comments of the next_lcg_state* functions below contain the
// "Source" column of a table in the following Wikipedia article on
// Linear congruential generators, in the section "Parameters in
// common use".

// https://en.wikipedia.org/wiki/Linear_congruential_generator


uint32
next_lcg_state1 (void)
{
    // Numerical Recipes
    lcg_state = (1664525 * lcg_state) + 1013904223;
    return lcg_state;
}


uint32
next_lcg_state2 (void)
{
    // Borland C/C++
    lcg_state = (22695477 * lcg_state) + 1;
    return lcg_state;
}


uint32
next_lcg_state3 (void)
{
    // C99, C11: Suggestion in the ISO/IEC 9899
    lcg_state = (1103515245 * lcg_state) + 12345;
    return lcg_state;
}


uint32
next_lcg_state4 (void)
{
    // Borland Delphi, Virtual Pascal
    lcg_state = (134775813 * lcg_state) + 1;
    return lcg_state;
}


uint32
next_lcg_state5 (void)
{
    // Microsoft Visual/Quick C/C++
    lcg_state = (214013 * lcg_state) + 2531011;
    return lcg_state;
}


#define next_lcg_state  next_lcg_state5


void
perfect_repeat (uint32 num_bits)
{
    uint32 repeat_len;
    uint32 mask;
    uint32 buf_size;
    uint32 *first_rand_nums;
    uint32 i;
    uint64 n;
    uint32 all_good;
    uint32 first_state;
    uint32 rand_state;
    uint32 rand_num;

    // num_bits is the # of least significant bits of the LCG that
    // will be returned as # 'the random number', in the range 0 thru
    // (2^num_bits) - 1.

    repeat_len = 1 << num_bits;
    mask = repeat_len - 1;

    buf_size = (unsigned) repeat_len * sizeof(uint32);
    first_rand_nums = (uint32 *) malloc(buf_size);
    if (first_rand_nums == NULL) {
        printf("Failed to allocate %u bytes for first_rand_nums\n", buf_size);
        exit(1);
    }

    first_state = init_lcg_state();

    n = 0;
    for (i = 0; i < repeat_len; i++) {
        ++n;
        rand_state = next_lcg_state();
        rand_num = rand_state & mask;
        first_rand_nums[i] = rand_num;
    }

    i = 0;
    all_good = FALSE;
    while (1) {
        ++n;
        rand_state = next_lcg_state();
        if (rand_state == first_state) {
            all_good = TRUE;
            break;
        }
        rand_num = rand_state & mask;
        if (rand_num != first_rand_nums[i]) {
            printf("n=%10lu rand_state=%10u rand_num=%10u != %10u=first_rand_nums[%u]\n",
                   n, rand_state, rand_num, first_rand_nums[i], i);
            break;
        }
        ++i;
        if (i == repeat_len) {
            i = 0;
        }
    }
    if (all_good) {
        printf("success for num_bits=%u n=%10lu\n", num_bits, n);
    }
}


int
main (int argc, char *argv[])
{
    uint32 num_bits;
    
    printf("sizeof(uint32)=%lu\n", sizeof(uint32));
    printf("sizeof(uint64)=%lu\n", sizeof(uint64));
    assert (sizeof(uint32) == 4);
    assert (sizeof(uint64) == 8);

    for (num_bits = 1; num_bits <= 26; num_bits++) {
        perfect_repeat(num_bits);
    }
}

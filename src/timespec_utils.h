#pragma once

#include <time.h>
#include <stdbool.h>

/*
TODO: Support Negative Carry
-10.6 - 1.6

-10 - 1 = -11
-0.6 - 0.6 = -1.2

-12.2
*/

void timespec_add(
    const struct timespec *ts_left,
    const struct timespec *ts_right,
    struct timespec *ts_result);

void timespec_subtract(
    const struct timespec *ts_left,
    const struct timespec *ts_right,
    struct timespec *ts_result);

// returns ts_left > ts_right
bool timespec_compare(
    const struct timespec *ts_left,
    const struct timespec *ts_right);

#include "timespec_utils.h"

#define NSEC_PER_SEC (1000000000L)

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
    struct timespec *ts_result)
{
    time_t sec = ts_left->tv_sec + ts_right->tv_sec;
    long nsec  = ts_left->tv_nsec + ts_right->tv_nsec;

    // negative carry: sec is positive nsec is negative(calculate)
    // 11.1 - 10.5 = 0.-4
    // sec = 1
    // nsec = 1-5 = -4
    if (sec > 0 && nsec < 0) {
        nsec += NSEC_PER_SEC;
        sec--;
    }

    // negative carry: sec is negative nsec is negative(overflow)
    if (sec < 0 && nsec < -NSEC_PER_SEC) {
        nsec += NSEC_PER_SEC;
        sec--;
    }


    // positive carry(overflow)
    if (nsec >= NSEC_PER_SEC) {
        nsec -= NSEC_PER_SEC;
        sec++;
    }

    ts_result->tv_sec  = sec;
    ts_result->tv_nsec = nsec;
}


void timespec_subtract(
    const struct timespec *ts_left,
    const struct timespec *ts_right,
    struct timespec *ts_result)
{
    time_t sec;
    long nsec;

    sec = ts_left->tv_sec - ts_right->tv_sec;
    nsec = ts_left->tv_nsec - ts_right->tv_nsec;

    if (ts_left->tv_sec >= 0 && ts_left->tv_nsec >=0) {
        if ((sec < 0 && nsec > 0) || (sec > 0 && nsec >= NSEC_PER_SEC)) {
            nsec -= NSEC_PER_SEC;
            sec++;
        }

        if (sec > 0 && nsec < 0) {
            nsec += NSEC_PER_SEC;
            sec--;
        }
    } else {
        if (nsec <= -NSEC_PER_SEC || nsec >= NSEC_PER_SEC) {
            nsec += NSEC_PER_SEC;
            sec--;
        }
        if ((sec < 0 && nsec > 0)) {
            nsec -= NSEC_PER_SEC;
            sec++;
        }
    }

    ts_result->tv_sec = sec;
    ts_result->tv_nsec = nsec;
}

// returns ts_left > ts_right
bool timespec_compare(
    const struct timespec *ts_left,
    const struct timespec *ts_right)
{
    if (ts_left->tv_sec > ts_right->tv_sec)
        return true;
    if (ts_left->tv_sec < ts_right->tv_sec)
        return false;
    if (ts_left->tv_nsec > ts_right->tv_nsec)
        return true;
    return false;
}

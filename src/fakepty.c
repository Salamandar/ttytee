#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <log.h>

#include "tee.c"


bool timespec_bigger(const struct timespec *a, const struct timespec *b) {
    if (a->tv_sec > b->tv_sec) {
        return true;
    }
    if (a->tv_sec < b->tv_sec) {
        return false;
    }
    if (a->tv_nsec > b->tv_nsec) {
        return true;
    }
    return false;
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        log_fatal("usage: %s <fake pty>", argv[0]);
        exit(1);
    }
    const char *pty_name = argv[1];

    struct pollfd poll_pty = {
        .fd = 0,
        .events = POLLIN
    };

    if (!create_pty(pty_name, &poll_pty.fd, true)) {
        log_fatal("Could not create pty!");
        exit(1);
    }

    log_info("You can now open pty at %s", pty_name);

    struct timespec next_send_time, now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &next_send_time);
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);


    while (true) {
        int poll_res = poll(&poll_pty, 1, 1000);
        if (poll_res > 0 && poll_pty.revents) {
            printf("revents = %d\n", poll_pty.revents);
            if (poll_pty.revents == POLLHUP) {
                printf("PTY closed, reopening...\n");
                if (!create_pty(pty_name, &poll_pty.fd, true)) {
                    log_fatal("Could not create pty!");
                    exit(1);
                }
                continue;
            }

            readout_count = read(poll_pty.fd, readout_buffer, BUFFER_SIZE);
            if (readout_count == -1) {
                log_error("Could not read pty: %s", strerror(errno));
            } else {
                readout_buffer[readout_count] = '\0';
                printf("Received from pty: %s\n", readout_buffer);
            }
        }

        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        if (timespec_bigger(&now, &next_send_time)) {
            next_send_time = now;
            next_send_time.tv_sec++;
            write(poll_pty.fd, "pouet\n", 7);
        }
    }



}

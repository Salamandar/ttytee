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

#include "timespec_utils.h"
#include "tee.h"

#define BUFFER_SIZE 1024

void sig_handler(int sig) {
    (void)sig;
    printf("Exiting.\n");
    exit(0);
}

char readout_buffer[BUFFER_SIZE];
ssize_t readout_count = 0;
char writein_buffer[BUFFER_SIZE];
ssize_t writein_count = 0;

// Just useful for clear_tee
size_t ptys_count = 0;
const char **ptys;

bool open_tty(const char* tty_name, int *tty_fd) {
    log_debug("Opening TTY %s...", tty_name);
    *tty_fd = open(tty_name, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
    if (*tty_fd == -1) {
       log_fatal("Could not open the tty %s: %s.", tty_name, strerror(errno));
       return false;
    }
    return true;
}

bool create_pty(const char* pty_name, int *pty_master_fd, bool overwrite) {
    if (overwrite && !faccessat(AT_FDCWD, pty_name, F_OK, AT_SYMLINK_NOFOLLOW)) {
        log_debug("Wiping existing file for PTY %s...", pty_name);
        if (remove(pty_name) == -1) {
            log_fatal("Could not overwrite the pty %s: %s", pty_name, strerror(errno));
            return false;
        }
    }

    log_debug("Creating PTY %s...", pty_name);

    *pty_master_fd = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (*pty_master_fd == -1) {
        log_fatal("Could not open master PTY: %s", strerror(errno));
        return false;
    }

    struct termios termios_p;
    if (tcgetattr(*pty_master_fd, &termios_p) == -1) {
        log_fatal("Could not get PTY settings: %s", strerror(errno));
        return false;
    }
    termios_p.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    if (tcsetattr(*pty_master_fd, TCSANOW, &termios_p) == -1) {
        log_fatal("Could not set PTY settings: %s", strerror(errno));
        return false;
    }

    if (grantpt(*pty_master_fd) == -1) {
        log_fatal("Could not set permissions of slave PTY: %s", strerror(errno));
        return false;
    }
    if (unlockpt(*pty_master_fd) == -1) {
        log_fatal("Could not unlock slave PTY: %s", strerror(errno));
        return false;
    }

    char pty_slave_name[PATH_MAX];
    int res = ptsname_r(*pty_master_fd, pty_slave_name, PATH_MAX);
    if (res) {
        log_fatal("Could not get the name of the slave PTY: %s", strerror(res));
        return false;
    }

    if (symlink(pty_slave_name, pty_name) == -1) {
        log_fatal("Could not create the pty symlink %s: %s.", pty_name, strerror(errno));
        return false;
    }

    return true;
}

bool remove_pty(const char* pty_name) {
    log_debug("Removing PTY %s...", pty_name);
    if (remove(pty_name) == -1) {
       log_fatal("Could not remove the pty %s: %s.", pty_name, strerror(errno));
       return false;
    }
    return true;
}

bool read_to_buffer(const struct pollfd *fd, char *buffer);

bool has_bitmask(short value, short bitmask) {
    return (value & bitmask) == bitmask;
}


bool run_tee(const char* tty, const char* _ptys[], size_t _ptys_count, bool overwrite_ptys) {
    // Populate global variables for clear_tee
    ptys = _ptys;

    // List of FDs to poll. First one is the TTY, the others are the PTYs.
    struct pollfd poll_fds[ptys_count + 1];

    // Handle ctrl-c
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // Open existing TTY
    if (!open_tty(tty, &poll_fds[0].fd)) {
        return false;
    }

    // Create PTYs
    for (size_t i = 0; i < _ptys_count; i++) {
        if (!create_pty(ptys[i], &poll_fds[i+1].fd, overwrite_ptys)) {
            return false;
        }
        log_debug("POLLIN = %d, pty %s events = %d", POLLIN, ptys[i], poll_fds[i+1].events);
        // Increment global variable for clear_tee
        ptys_count++;
    }

    // Setup poll_fds events
    for (size_t i = 0; i < ptys_count + 1; i++) {
        poll_fds[i].events = POLLIN;
    }

    struct timespec last_poll, now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &last_poll);

    while (true) {
        int poll_res = poll(poll_fds, ptys_count + 1, 1000);

        // If the last loop + the poll took less than 1ms, we should sleep 10ms
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        struct timespec delay = { .tv_sec = 0, .tv_nsec = 10000000 };
        timespec_add(&last_poll, &delay, &last_poll);
        if (!timespec_compare(&now, &last_poll)) {
            usleep(10000);
            clock_gettime(CLOCK_MONOTONIC_RAW, &last_poll);
            last_poll = now;
            log_debug("sleep");
        }

        if (poll_res == 0) {
            log_debug("poll timed out");
            continue;
        }
        if (poll_res < 0) {
            log_error("poll failed: %s", strerror(errno));
            continue;
        }

        // Read TTY
        if (has_bitmask(poll_fds[0].revents, POLLIN)) {
            readout_count = read(poll_fds[0].fd, readout_buffer, BUFFER_SIZE);
            if (readout_count == -1) {
                log_error("Could not read tty: %s", strerror(errno));
            }
        }

        // Read PTYs
        for (size_t i = 1; i < ptys_count + 1; i++) {
            if (poll_fds[i].revents == 0) {
                continue;
            }
            if (has_bitmask(poll_fds[i].revents, POLLIN)) {
                ssize_t writein_more = read(poll_fds[i].fd, writein_buffer + writein_count, BUFFER_SIZE - writein_count);

                if (writein_more == -1) {
                    log_error("Could not read PTY %s", ptys[i-1]);
                    continue;
                }
                writein_count += writein_more;
            }
            if(has_bitmask(poll_fds[i].revents, POLLERR)) {
                log_info("pty %s was closed, reopening...", ptys[i-1]);
                if (!create_pty(ptys[i-1], &poll_fds[i].fd, true)) {
                    log_fatal("Could not recreate pty %s!", ptys[i-1]);
                    return false;
                }
                continue;
            }
        }

        log_debug("tty_readount_count = %d", readout_count);

        // Now write to everyone
        ssize_t written = write(poll_fds[0].fd, writein_buffer, writein_count);
        if (written == -1) {
            log_error("Could not write in TTY: %s", strerror(errno));
        } else if (written != writein_count) {
            log_warn("Could not write everything in TTY (TODO) but %d bytes", written);
        }
        writein_count = 0;

        for (size_t i = 1; i < ptys_count + 1; i++) {
            written = write(poll_fds[i].fd, readout_buffer, readout_count);
            log_debug("written %d into %s", written, ptys[i-1]);
            if (written == -1) {
                log_error("Could not write in PTY %s: %s", ptys[i-1], strerror(errno));
            } else if (written != readout_count) {
                log_warn("Could not write everything to PTY %s (TODO) but %d bytes", ptys[i-1], written);
            }
        }
        readout_count = 0;

    }


    return true;
}

void clear_tee(void) {
    for (size_t i = 0; i < ptys_count; i++) {
        remove_pty(ptys[i]);
    }
}

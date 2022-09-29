#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <log.h>

#include "tee.h"

#define BUFFER_SIZE 1024

void sig_handler(int sig) {
    (void)sig;
    printf("Exiting.\n");
    exit(0);
}

char readout_buffer[BUFFER_SIZE];
char writein_buffer[BUFFER_SIZE];

// Just useful for clear_tee
size_t ptys_count = 0;
const char **ptys;

bool open_tty(const char* tty_name, int *tty_fd) {
    log_debug("Opening TTY %s...", tty_name);
    *tty_fd = open(tty_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (*tty_fd == -1) {
       log_fatal("Could not open the tty %s: %s.", tty_name, strerror(errno));
       return false;
    }
    return true;
}

bool create_pty(const char* pty_name, int *pty_master_fd, bool overwrite) {
    if (overwrite && !access(pty_name, F_OK)) {
        log_debug("Wiping existing file for PTY %s...", pty_name);
        if (remove(pty_name) == -1) {
            log_fatal("Could not overwrite the pty %s: %s", pty_name, strerror(errno));
            return false;
        }
    }

    log_debug("Creating PTY %s...", pty_name);

    *pty_master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (*pty_master_fd == -1) {
        log_fatal("Could not open master PTY: %s", strerror(errno));
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
    if (ptsname_r(*pty_master_fd, pty_slave_name, PATH_MAX)) {
        log_fatal("Could not get the name of the slave PTY: %s", strerror(errno));
        return false;
    }

    if (symlink(pty_slave_name, pty_name) == -1) {
        log_fatal("Could not create the pty %s: %s.", pty_name, strerror(errno));
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


bool run_tee(const char* tty, const char* _ptys[], size_t _ptys_count, bool overwrite_ptys) {
    // Populate global variables for clear_tee
    ptys = _ptys;

    int tty_fd;
    int pty_fds[ptys_count];

    // Handle ctrl-c
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // Open existing TTY
    if (!open_tty(tty, &tty_fd)) {
        return false;
    }

    // Create PTYs
    for (size_t i = 0; i < _ptys_count; i++) {
        if (!create_pty(ptys[i], &pty_fds[i], overwrite_ptys)) {
            return false;
        }
        // Increment global variable for clear_tee
        ptys_count++;
    }

    while (true) {
        sleep(1);
    }


    return true;
}

void clear_tee(void) {
    for (size_t i = 0; i < ptys_count; i++) {
        remove_pty(ptys[i]);
    }
}

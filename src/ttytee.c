#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <argparse.h>
#include <log.h>

#define PROJECT_NAME "ttytee"

static const char *const usages[] = {
    PROJECT_NAME " -t tty pty pty pty",
    NULL,
};
const char *arg_tty_path = NULL;
bool arg_overwrite = false;
struct argparse_option options[] = {
    OPT_HELP(),
    OPT_STRING('t', "tty", &arg_tty_path, "tty to copy", NULL, 0, 0),
    OPT_BIT('o', "overwrite", &arg_overwrite, "overwrite PTYs if they already exist", NULL, true, OPT_NONEG),
    OPT_END(),
};

int main(int argc, const char **argv) {
    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(
        &argparse,
        "\nThis utility creates multiple PTYs connected to the same TTY.",
        "\nPTYs are passed as \"non-options\", that means without --pty."
    );
    argc = argparse_parse(&argparse, argc, argv);

    if (!arg_tty_path) {
        printf("No TTY given to copy.\n\n");
        argparse_usage(&argparse);
        exit(1);
    }
    if (argc == 0) {
        printf("No PTY given to create.\n\n");
        argparse_usage(&argparse);
        exit(1);
    }

    printf("path: %s\n", arg_tty_path);
    printf("Overwrite %s\n", arg_overwrite ? "enabled" : "disabled");
    for (int i = 0; i < argc; i++) {
        printf("pty %d: %s\n", i, *(argv + i));
    }

    return 0;
}

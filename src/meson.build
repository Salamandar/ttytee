
ttytee = executable('ttytee',
    'ttytee.c',
    'tee.c',
    'timespec_utils.c',
    dependencies: [
        argparse,
        log_c,
    ],
    install: true,
)

fakepty = executable('fakepty',
    'fakepty.c',
    'timespec_utils.c',
    dependencies: [
        argparse,
        log_c,
    ],
    install: false,
)

test('basic', ttytee)

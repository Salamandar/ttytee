project('ttytee',
    'c',
    version: '0.1',
    meson_version: '>=0.56.0',
    default_options: [
        'c_std=gnu2x',
        'warning_level=3'
    ]
)


argparse = subproject('argparse').get_variable('argparse')
log_c = subproject('log.c').get_variable('log_c')



exe = executable('ttytee',
    'ttytee.c',
    dependencies: [
        argparse,
        log_c,
    ],
    install: true,
)

test('basic', exe)

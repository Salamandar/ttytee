project('ttytee',
    'c',
    version: '0.1',
    meson_version: '>=0.52.0',
    default_options: [
        'c_std=gnu11',
        'warning_level=3'
    ]
)

add_project_arguments([
    '-D_XOPEN_SOURCE=700',
    '-D_GNU_SOURCE',
], language: 'c')

argparse = subproject('argparse').get_variable('argparse')
log_c = subproject('log.c').get_variable('log_c')

subdir('src')

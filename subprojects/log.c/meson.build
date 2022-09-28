project('log.c', 'c')

inc = include_directories('log.c/src')

liblog_c = static_library('log',
    'log.c/src/log.c',
    include_directories: inc,
    c_args: [
        '-DLOG_USE_COLOR',
    ],
    install: false
)

log_c = declare_dependency(
    include_directories: inc,
    link_with: liblog_c,
)

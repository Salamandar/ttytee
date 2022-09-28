project('argparse', 'c')

inc = include_directories('argparse')

libargparse = static_library('argparse',
    'argparse/argparse.c',
    include_directories: inc,
    install: false
)

argparse = declare_dependency(
    include_directories: inc,
    link_with: libargparse
)

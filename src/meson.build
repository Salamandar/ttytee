
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

cargo_options = [ '--manifest-path', meson.source_root() / 'Cargo.toml' ]
cargo_options += [ '--target-dir', meson.current_build_dir() ]
if get_option('profile') == 'default'
    cargo_options += [ '--release' ]
    rust_target = 'release'
    message('Building in release mode')
else
    rust_target = 'debug'
    message('Building in debug mode')
endif
cargo_env = [ 'CARGO_HOME=' + meson.build_root() / 'cargo-home' ]

cargo_build = custom_target(
    'cargo-build',
    build_by_default: true,
    build_always_stale: true,
    output: meson.project_name() + '_rs',
    console: true,
    install: true,
    install_dir: get_option('bindir'),
    # depends: resources,
    command: [
        'env',
        cargo_env,
        cargo, 'build',
        cargo_options,
        '&&',
        'cp', 'src' / rust_target / meson.project_name(), '@OUTPUT@',
    ]
)

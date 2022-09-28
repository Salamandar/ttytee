# TTYTee

This utility connects an already existing TTY to multiple new PTYs.

It's useful to have multiple softwares connected to a single TTY, or to just dump everything coming from the TTY.

## Building

This project uses Meson.
```
meson _build --buildtype release
cd _build
ninja
DESTDIR=... ninja install

```


## Usage

```
ttytee -t the_existing_tty new_pty1 new_pty2 etc
```

##

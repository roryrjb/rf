# rf

> A tiny and simple cross-platform file finder

## Installation

### Platform Support

Ideally all the following platforms will eventually be supported:

- [X] Linux glibc
- [X] Linux musl
- [X] macOS
- [ ] Win32
- [X] FreeBSD
- [X] OpenBSD
- [ ] NetBSD
- [ ] DragonFlyBSD
- [ ] illumos
- [ ] Haiku

### From Source

__Requirements:__

Minimum requirements are a C99 compiler and a POSIX-like environment. The build
system is `make`, any flavour _should_ be fine. For man pages you will require
[__scdoc__(1)](https://sr.ht/~sircmpwn/scdoc/).

__Building:__

Unless you have custom requirements, just use `make`, the following options
are available:

    $ make              # -> dynamic binary
    $ make static       # -> static binary
    # make install      # -> install dynamic binary and man pages to PREFIX

## Usage

### Command Line

Let's start with a few simple examples.

If you want to find all `.c` files recursively from the current directory:

    $ rf \*.c

This is essentially a shortened version of the traditional `find . -name \*.c`.
Underneath rf uses [`fnmatch`](https://man.openbsd.org/fnmatch) so all the usual
glob rules apply. You can also use substring matching instead, something like:

    $ rf -s hello

This would match any files with 'hello' any where in the name. Although this is
less flexible, it can potentially make things easier and faster depending on
the particular use case.

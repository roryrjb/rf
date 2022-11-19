# rf

> A tiny and simple cross-platform file finder

## Installation

### Platform Support

Following platforms are supported:

* Linux glibc/musl
* macOS
* Win32
* FreeBSD
* OpenBSD

### From Source

__Requirements:__

Minimum requirements are a C99 compiler.

__Building on POSIX:__

```
$ make
```

__Building on Windows:__

Setup your environment with `vcvars64.bat`, then:

```
> make
```

## Usage

### Command Line

Let's start with a few simple examples.

If you want to find all `.c` files recursively from the current directory:

    rf *.c

rf uses [`fnmatch`](https://man.openbsd.org/fnmatch)/[`PathMatchSpecA`](https://learn.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-pathmatchspeca) so all the usual
glob rules apply. You can also use substring matching instead, something like:

    rf -s hello

This would match any files with 'hello' anywhere in the name. Although this is
less flexible, it can potentially make things easier and faster depending on
the particular use case.

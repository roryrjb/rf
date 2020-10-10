# rf-vim

> Simple `rf` Vim integration

### Installation

__Manual installation:__

If you are using default Vim paths (so not Neovim for example) you can
just run `make install`. Otherwise you should just be able to modify your
`runtimepath` and add `~/.vim/after/`.

__Using vim-plug, Vundle, et al:__

```viml
Plug 'roryrjb/rf', { 'rtp': 'contrib/vim' }  " vim-plug
Plugin 'roryrjb/rf', { 'rtp': 'contrib/vim' }  " Vundle
```

Other plugin systems will likely have very similar options. In a nutshell
the vim plugin provided here isn't in the root of the repo and assumes
you have `rf` already installed and present in your `$PATH`.

### Usage

The plugin provides two functions:

__RF:__

This function runs a getchar() loop, allowing you to search from the
root of the current working directory, using only substring patterns,
the idea being that it is very quick and simple to use, rather than
exposing more of the functionality that rf provides.

I usually bind the function like so:

```
map <c-p> :RF <CR>
```

The function is very simple and will call rf with your pattern
*at every key stroke*. Usually this is performant enough, but
with extremely large directory trees, you may have to wait several
seconds for results, which isn't great. Aside from any improvements
to the implementation, just make sure that any `.rfignore` files,
whether local or global are ignoring everything that can be ignored.

__RFGrep:__

```
:RFGrep search-pattern filename-pattern
```

This function is a wrapper around `grep -r` and rf. It allows you
to grep recursively over a smaller subset of files. Again the
implementation is very simple but with very large directory trees
or with filename patterns that match a lot of files you may
hit the argument limit for grep.

### Issues

Aside from the above mentioned limitations and issues, if you
are using anything other than a "Bourne" shell (bash, ksh, zsh, sh)
the functions may not work, in that case you may want to explictly:

```
set shell=sh
```

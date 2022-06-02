rf(1)

# NAME

rf - a tiny and simple file finder

# SYNOPSIS

*rf* [*-d* _directory_] [*pattern* _..._]

# DESCRIPTION

*rf* will find files or directories based on one or more glob (or optionally
substring) patterns, while respecting any ignore rules in *rfignore* files.

# OPTIONS

*-d* directory
	Search from specified directory, otherwise defaults to current working directory.

*-l* count
	Limit to specified matches count.

*-s*
	Match using substrings instead of globs.

*-v*
	Invert matches.

*-w*
	Match wholename, including path.

# SEE ALSO

*rfignore*(5)

# COPYRIGHT

Copyright © 2019 - 2021 Rory Bradford <roryrjb@gmail.com>.

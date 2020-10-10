rfconfig(5)

# NAME

rfconfig - configure rf behaviour

# SYNOPSIS

$XDG_CONFIG_HOME/rf/config

# DESCRIPTION

An *rfconfig* file is used to override default behaviour and configure options.

# FORMAT

The config file format is very simple. Comments are only available for a whole
line and start with #. Any lines that begin with a space are ignored.
Each valid config line consists of a key value pair with the key and value
separated by an = character and optionally a single space either side.
Additional spaces will become part of either the key and/or value.

# OPTIONS

*symlinks*
	Read symlinks (value: "true" or "false")

*wholename*
	Match whole path (value "true" or "false")

*limit*
	Limit amount of results (value: a positive integer)

*wildcard*
	Define a custom wildcard character, used in place of \*
	(value: a single ascii character)

*unmatched error*
	Exit with non-zero exit code if there were no matches

# COPYRIGHT

Copyright © 2019 - 2021 Rory Bradford <roryrjb@gmail.com>.

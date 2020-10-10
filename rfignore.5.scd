rfignore(5)

# NAME

rfignore - ignore specified paths

# SYNOPSIS

$XDG_CONFIG_HOME/rf/ignore, $HOME/.rfignore, .rfignore

# DESCRIPTION

An *rfignore* file will be used by *rf* to ignore files and paths
when iterating through directories. Any matching patterns will not be displayed.

*rf* will look in all places specified in the synopsis for glob patterns
on each line and take them all into account.

# SEE ALSO

*glob*(7)

# COPYRIGHT

Copyright © 2019 - 2021 Rory Bradford <roryrjb@gmail.com>.

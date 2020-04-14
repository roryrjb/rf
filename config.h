char *ignored_dirs[] = {"*node_modules*", "*.mypy_cache*", "*.git*", "*.hg*",
	"*__pycache__*", "*.eggs*"};

size_t ignored_dirs_size = sizeof(ignored_dirs) / sizeof(ignored_dirs[0]);

char *ignored_extensions[] = {"*.pyc", "*.jpg", "*.jpeg", "*.png", "*.gif",
	"*.zip", "*.tar", "*.xz", "*.gz", "*.gzip", "*.jar", "*.apk", "*.deb",
	"*.o"};

size_t ignored_extensions_size =
	sizeof(ignored_extensions) / sizeof(ignored_extensions[0]);

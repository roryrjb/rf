#ifndef RF_IGNORE_H
#define RF_IGNORE_H

#define IGNORE_SIZE 100

#ifdef _WIN32
#define MAXPATHLEN 1024
#endif

struct ignores {
	char **list;
	int size;
};

struct ignores *init_ignores(char *path);

void free_ignores(struct ignores *ignores);

#endif

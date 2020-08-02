#ifndef RF_IGNORE_H
#define RF_IGNORE_H

#define IGNORE_SIZE 100

struct ignores {
	char **list;
	int size;
};

struct ignores *init_ignores(char *path);

void free_ignores(struct ignores *ignores);

#endif

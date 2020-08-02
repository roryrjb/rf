
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 600

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ignore.h"

static int total_size;

struct ignores *init_ignores(char *path) {
	total_size = IGNORE_SIZE;
	struct ignores *ignores = calloc(sizeof(struct ignores), 1);
	ignores->list = calloc(sizeof(char *), IGNORE_SIZE);
	ignores->size = 0;

	int i = 0;
	FILE *ignore = fopen(path, "r");
	char *line = NULL;
	size_t llen = 0;
	ssize_t r;

	if (ignore != NULL) {
		while ((r = getline(&line, &llen, ignore)) != -1) {
			char *l = calloc(sizeof(char *), strlen(line) - 1);

			for (size_t j = 0, k = 0; j < strlen(line); j++) {
				char c = line[j];

				if (isspace(c)) {
					break;
				}

				l[k++] = c;
			}

			if (i + 1 > total_size) {
				ignores->list = realloc(
					ignores->list, sizeof(char *) * (total_size + IGNORE_SIZE));
				total_size += IGNORE_SIZE;
			}

			ignores->list[i++] = l;
		}

		free(line);
		fclose(ignore);
	}

	ignores->size = i;

	return ignores;
}

void free_ignores(struct ignores *ignores) {
	if (ignores->size > 0) {
		for (int i = 0; i < ignores->size; i++) {
			free(ignores->list[i]);
		}
	}

	free(ignores->list);
	free(ignores);
}

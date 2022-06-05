
#define _XOPEN_SOURCE 700

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rf.h"
#include "ignore.h"

static int total_size;

struct ignores *init_ignores(char *path) {
	int i = 0;
	FILE *ignore = fopen(path, "r");

	if (ignore == NULL) {
		return NULL;
	}

	char line[MAX_PATH_LENGTH];
	total_size = IGNORE_SIZE;
	struct ignores *ignores =
		(struct ignores *)calloc(sizeof(struct ignores), 1);
	ignores->list = (char **)calloc(sizeof(char *), IGNORE_SIZE);
	ignores->size = 0;

	while (fgets(line, MAX_PATH_LENGTH, ignore) != NULL) {
		/** isspace doesn't necessarily hold true for `\n` for anything
		 * other than the 'C' locale on some platforms, so we have to do
		 * an additional check for this
		 */

		if (strlen(line) == 1 && (isspace(line[0]) || line[0] == '\n')) {
			continue;
		}

		if (strlen(line) > 0) {
			char *l = (char *)calloc(sizeof(char), strlen(line));

			if (l == NULL) {
				fprintf(stderr, "calloc\n");
				exit(EXIT_FAILURE);
			}

			for (size_t j = 0, k = 0; j < strlen(line); j++) {
				char c = line[j];

				if (isspace(c) || c == '\n') {
					break;
				}

				l[k++] = c;
			}

			if (i + 1 > total_size) {
				ignores->list = (char **)realloc(
					ignores->list, sizeof(char *) * (total_size + IGNORE_SIZE));
				total_size += IGNORE_SIZE;
			}

			ignores->list[i++] = l;
		}
	}

	fclose(ignore);

	ignores->size = i;

	return ignores;
}

void free_ignores(struct ignores *ignores) {
	if (ignores != NULL) {
		if (ignores->size > 0) {
			for (int i = 0; i < ignores->size; i++) {
				free(ignores->list[i]);
			}
		}

		free(ignores->list);
		free(ignores);
	}
}

#define _XOPEN_SOURCE 700

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char config_key[KV_STRING_LIMIT] = {'\0'};
char config_value[KV_STRING_LIMIT] = {'\0'};

int config_get(size_t *llen, FILE *fp) {
	int i = 0;
	int k = 0;
	int v = 0;
	int len = 0;
	int in_key = 1;
	char *line = NULL;

	memset(config_key, '\0', KV_STRING_LIMIT);
	memset(config_value, '\0', KV_STRING_LIMIT);

	if (getline(&line, llen, fp) < 0) {
		free(line);
		return -1;
	}

	len = strlen(line);

	switch (line[0]) {
	case '=':
	case '#':
		free(line);
		return 0;

	default:
		if (isspace(line[0])) {
			free(line);
			return 0;
		}
	}

	for (i = 0; i < len; i++) {
		char c = line[i];

		switch (c) {
		case '=':
			if (in_key == 1) {
				in_key = 0;
				break;
			}

			/* fallthrough */
		default:
			if (c != '\n') {
				if (isspace(c)) {
					/* if previous char was '=' then skip */
					if (i > 0 && line[i - 1] == '=') {
						break;
					}

					/* if next char is '=' then skip */
					if (i + 1 < len && line[i + 1] == '=') {
						break;
					}
				}

				if (in_key) {
					config_key[k++] = c;
				} else {
					config_value[v++] = c;
				}
			}

			break;
		}
	}

	free(line);

	return 0;
}

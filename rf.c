#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"

extern char *__progname;

struct switches {
	int substring;
	int invert;
	int limit;
	int count;
	int unlink;
	char *cmd;
};

static void usage(char *error) {
	if (error != NULL) {
		fprintf(stderr, "Error: %s\n\n", error);
	}

	fprintf(stderr, "usage: %s [-lsvSU] pattern ...\n", __progname);
}

static int is_child(char *dirname) {
	if (strcmp("..", dirname) == 0 || strcmp(".", dirname) == 0) {
		return 0;
	}

	return 1;
}

static int not_in_array(char **arr, char *dirname, size_t size) {
	for (int i = 0; i < size; i++) {
		if (fnmatch(arr[i], dirname, 0) == 0) {
			return 0;
		}
	}

	return 1;
}

static int excluded_extension(char *filename) {
	for (int i = 0; i < ignored_extensions_size; i++) {
		int res = fnmatch(ignored_extensions[i], filename, 0);

		if (res == 0) {
			return 1;
		}
	}

	return 0;
}

static void handle_result(
	char *path, struct switches *switches, struct dirent *entry) {
	int i, j, k = 0;
	char cmd[MAXPATHLEN];
	char full_path[MAXPATHLEN];

	memset(cmd, '\0', MAXPATHLEN);
	full_path[0] = '\0';

	strcat(full_path, path);
	strcat(full_path, "/");
	strcat(full_path, entry->d_name);

	if (is_child(entry->d_name) != 0) {
		if (switches->unlink) {
			int r = unlink(full_path);

			if (r < 0) {
				perror("unlink");
			} else {
				printf("removed '%s'\n", full_path);
			}
		} else if (switches->cmd != NULL) {
			int l = strlen(switches->cmd);

			for (i = 0, j = 0; i < l; i++) {
				char c = switches->cmd[i];

				if (c == '%' && (i + 1 < l) && switches->cmd[i + 1] == 's') {
					i++;

					for (k = 0; k < strlen(full_path); k++) {
						cmd[j++] = full_path[k];
					}
				} else {
					cmd[j++] = c;
				}
			}

			system(cmd);
		} else {
			printf("%s\n", full_path);
		}
	}
}

/* return 1 if breaking early (e.g. reaching limit) otherwise return 0 */
static int recurse_find(char **patterns, int *pattern_count, char *dirname,
	struct switches *switches) {
	DIR *dir;

	char path[MAXPATHLEN] = {'\0'};
	int break_early = 0;
	strcat(path, dirname);
	dir = opendir(path);

	if (dir != NULL && not_in_array(ignored_dirs, dirname, ignored_dirs_size)) {
		struct dirent *entry;

		while ((entry = readdir(dir)) != NULL) {
			int matched = switches->invert ? 1 : 0;
			int p = 0;

			switch (entry->d_type) {
			case DT_DIR:
				if (is_child(entry->d_name) &&
					not_in_array(
						ignored_dirs, entry->d_name, ignored_dirs_size)) {
					char child_path[MAXPATHLEN] = {'\0'};
					strcat(child_path, path);
					strcat(child_path, "/");
					strcat(child_path, entry->d_name);
					if (recurse_find(
							patterns, pattern_count, child_path, switches)) {
						break_early = 1;
						break;
					};
				}

				break;
			case DT_REG:
				if (excluded_extension(entry->d_name)) {
					matched = 0;
					break;
				}

				for (; p < *pattern_count; p++) {
					char *pattern = patterns[p];

					if (switches->substring) {
						if (strstr(entry->d_name, pattern) != NULL) {
							matched = switches->invert ? 0 : 1;
						}
					} else {
						if (fnmatch(pattern, entry->d_name, 0) == 0) {
							matched = switches->invert ? 0 : 1;
						}
					}
				}

				break;
			default:
				break;
			} /* switch */

			if (break_early) {
				closedir(dir);
				return 1;
			}

			if (matched) {
				handle_result(path, switches, entry);

				if (switches->limit > 0 &&
					++switches->count == switches->limit) {
					closedir(dir);
					return 1;
				}
			}
		} /* while */

		closedir(dir);
	} /* if */

	return 0;
}

int main(int argc, char **argv) {
	/* printing switches */
	int substring = 0;
	int invert = 0;
	int limit = 0;
	int printing = 0;

	/* operating switches */
	int unlink = 0;
	char *cmd;

	int count = 0; /* used to count how matches we find */
	int ch;

	char *remainder;

	while ((ch = getopt(argc, argv, "l:svS:U")) > -1) {
		switch (ch) {
		case 'h':
			usage(NULL);
			exit(EXIT_SUCCESS);

		case 's':
			substring = 1;
			break;

		case 'v':
			invert = 1;
			break;

		case 'S':
			if (system(NULL) == 0) {
				fprintf(stderr, "A shell isn't available.");
				exit(EXIT_FAILURE);
			}

			cmd = optarg;

			break;

		case 'l':
			limit = strtol(optarg, &remainder, 10);

			if (limit < 0) {
				usage("Invalid limit.");
				exit(EXIT_FAILURE);
			}

			break;

		case 'U':
			unlink = 1;
			break;
		}
	}

	/* sanity check opts for conflicts */
	printing = invert + limit;
	/* int operating = unlink; */

	if (unlink == 1 && printing > 0) {
		fprintf(stderr, "Cannot use -U with any of -lsv.\n");
		exit(EXIT_FAILURE);
	} else if (cmd != NULL && printing > 0) {
		fprintf(stderr, "Cannot use -S with any of -lsv.\n");
		exit(EXIT_FAILURE);
	}

	if (optind < argc) {
		int i = 0;
		struct switches switches;
		int pattern_count = argc - optind;
		char **patterns = malloc(sizeof(char *) * pattern_count);

		memset(patterns, '\0', optind);

		while (optind < argc) {
			patterns[i++] = argv[optind++];
		}

		switches.substring = substring;
		switches.invert = invert;
		switches.limit = limit;
		switches.count = count;
		switches.unlink = unlink;
		switches.cmd = cmd;

		if (recurse_find(patterns, &pattern_count, ".", &switches)) {
			/* finished early because we reached the limit */
		};

		free(patterns);
	} else {
		usage(NULL);
	}

	return 0;
}

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#define _XOPEN_SOURCE 700

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

#include "ignore.h"

extern char *__progname;

struct ignores *global_ignores;
struct ignores *config_ignores;
struct ignores *local_ignores;

struct switches {
	int substring;
	int wholename;
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

	fprintf(
		stderr, "usage: %s [-d directory] [-lsvw] pattern ...\n", __progname);
}

static int is_child(char *dirname) {
	if (strcmp("..", dirname) == 0 || strcmp(".", dirname) == 0) {
		return 0;
	}

	return 1;
}

static int excluded(char *name) {
	if (global_ignores != NULL) {
		for (int i = 0; i < global_ignores->size; i++) {
			int res = fnmatch(global_ignores->list[i], name, 0);

			if (res == 0) {
				return 1;
			}
		}
	}

	if (config_ignores != NULL) {
		for (int i = 0; i < config_ignores->size; i++) {
			int res = fnmatch(config_ignores->list[i], name, 0);

			if (res == 0) {
				return 1;
			}
		}
	}

	if (local_ignores != NULL) {
		for (int i = 0; i < local_ignores->size; i++) {
			int res = fnmatch(local_ignores->list[i], name, 0);

			if (res == 0) {
				return 1;
			}
		}
	}

	return 0;
}

/* return 1 if breaking early (e.g. reaching limit) otherwise return 0 */
static int recurse_find(char **patterns, int *pattern_count, char *dirname,
	struct switches *switches) {
	DIR *dir;

	char path[MAXPATHLEN] = {'\0'};
	strcat(path, dirname);
	dir = opendir(path);

	if (dir != NULL && !excluded(dirname)) {
		struct dirent *entry;

		while ((entry = readdir(dir)) != NULL) {
			int matched = 0;
			int p = 0;

			char full_path[MAXPATHLEN] = {'\0'};
			strcat(full_path, path);
			strcat(full_path, "/");
			strcat(full_path, entry->d_name);

			struct stat entry_stat;

			if (stat(full_path, &entry_stat)) {
				perror("stat");
				continue;
			}

			if (entry_stat.st_mode & S_IFDIR) {
				if (is_child(entry->d_name) && !excluded(entry->d_name)) {
					if (recurse_find(
							patterns, pattern_count, full_path, switches)) {
						closedir(dir);
						return 1;
					};
				}
			} else if (entry_stat.st_mode & S_IFREG) {
				if (excluded(entry->d_name)) {
					continue;
				}

				for (; p < *pattern_count; p++) {
					char *pattern = patterns[p];

					if (switches->substring) {
						if (strstr(
								switches->wholename ? full_path : entry->d_name,
								pattern) != NULL) {
							matched = 1;
						}
					} else {
						if (fnmatch(pattern,
								switches->wholename ? full_path : entry->d_name,
								0) == 0) {
							matched = 1;
						}
					}
				}

				if (switches->invert) {
					if (matched) {
						matched = 0;
					} else {
						matched = 1;
					}
				}
			}

			if (matched) {
				if (is_child(entry->d_name) != 0) {
					printf("%s\n", full_path);
				}

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
	struct switches switches = {
		.substring = 0, .invert = 0, .wholename = 0, .limit = 0, .count = 0};

	int ch;
	char *remainder;

	while ((ch = getopt(argc, argv, "l:svw")) > -1) {
		switch (ch) {
		case 'd':
			root = optarg;
			break;

		case 'h':
			usage(NULL);
			exit(EXIT_SUCCESS);

		case 's':
			switches.substring = 1;
			break;

		case 'v':
			switches.invert = 1;
			break;

		case 'w':
			switches.wholename = 1;
			break;

		case 'l':
			do {
				int limit = strtol(optarg, &remainder, 10);

				if (limit < 0) {
					usage("Invalid limit.");
					exit(EXIT_FAILURE);
				}
				switches.limit = limit;
			} while (0);

			break;
		}
	}

	char *home = getenv("HOME");
	char *xdg_config_home = getenv("XDG_CONFIG_HOME");
	char cwd[MAXPATHLEN];

	if (getcwd(cwd, MAXPATHLEN) == NULL) {
		perror("getcwd");
		exit(EXIT_FAILURE);
	}

	char global_ignore_path[(strlen(home) + strlen(".rfignore") + 1)];
	char config_ignore_path[xdg_config_home
								? (strlen(xdg_config_home) + strlen("ignore") +
									  3)
								: (strlen(cwd) + strlen(".rfignore") + 1)];
	char local_ignore_path[strlen(cwd) + strlen(".rfignore") + 1];
	sprintf(global_ignore_path, "%s/%s", home, ".rfignore");
	sprintf(local_ignore_path, "%s/%s", cwd, ".rfignore");

	if (xdg_config_home) {
		sprintf(config_ignore_path, "%s/rf/%s", xdg_config_home, "ignore");
	} else {
		sprintf(config_ignore_path, "%s/.config/rf/%s", home, "ignore");
	}

	global_ignores = init_ignores(global_ignore_path);
	config_ignores = init_ignores(config_ignore_path);
	local_ignores = init_ignores(local_ignore_path);

	if (optind < argc) {
		int i = 0;
		int pattern_count = argc - optind;
		char **patterns = (char **)calloc(sizeof(char *), pattern_count);

		memset(patterns, '\0', optind);

		while (optind < argc) {
			patterns[i++] = argv[optind++];
		}

		if (recurse_find(patterns, &pattern_count, root, &switches)) {
			/* finished early because we reached the limit */
		};

		free(patterns);
		free_ignores(global_ignores);
		free_ignores(config_ignores);
		free_ignores(local_ignores);
	} else {
		usage(NULL);
	}

	return 0;
}

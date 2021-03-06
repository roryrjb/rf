#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#define _XOPEN_SOURCE 700

#include <ctype.h>
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
#include "ignore.h"
#include "include/common/common.h"

#define IGNORE "ignore"
#define CONFIG "config"
#define RFIGNORE ".rfignore"

extern char *__progname;

struct ignores *global_ignores;
struct ignores *config_ignores;
struct ignores *local_ignores;

struct switches {
	int count;
	int invert;
	int limit;
	int substring;
	int wholename;
};

int read_links = 0;

static void usage(char *error) {
	if (error != NULL) {
		fprintf(stderr, "Error: %s\n\n", error);
	}

	fprintf(stderr,
		"usage: %s [-d directory] [-c config] [-lsvw] pattern ...\n",
		__progname);
}

static int is_child(char *dirname) {
	if (strcmp("..", dirname) == 0 || strcmp(".", dirname) == 0) {
		return 0;
	}

	return 1;
}

static int excluded(const char *name) {
	if (!fnmatch("/proc/*", name, 0)) {
		return 1;
	}

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

char *escape_string(char *input) {
	char ch;
	int i = 0;
	int j = 0;
	char *output = calloc(sizeof(char), strlen(input) * 2);

	while ((ch = input[i]) != '\0') {
		switch (ch) {
		case ' ':
		case '\t':
		case '\'':
		case '(':
		case ')':
			output[j++] = '\\';
			output[j++] = ch;
			i++;
			break;
		default:
			output[j++] = input[i++];
		}
	}

	return output;
}

int has_spaces(char *input) {
	char ch;
	int i = 0;

	while ((ch = input[i++]) != '\0') {
		if (isspace(ch)) {
			return 1;
		}
	}

	return 0;
}

/* return 1 if breaking early (e.g. reaching limit) otherwise return 0 */
static int recurse_find(char **patterns, int *pattern_count,
	const char *dirname, struct switches *switches) {
	DIR *dir;

	char path[MAXPATHLEN] = {'\0'};
	strlcat(path, dirname, MAXPATHLEN);
	dir = opendir(path);

	if (dir != NULL && !excluded(dirname)) {
		struct dirent *entry;

		while ((entry = readdir(dir)) != NULL) {
			int matched = 0;
			int p = 0;

			char full_path[MAXPATHLEN] = {'\0'};
			strlcat(full_path, path, MAXPATHLEN);

			if (full_path[strlen(full_path) - 1] != '/') {
				strlcat(full_path, "/", MAXPATHLEN);
			}

			strlcat(full_path, entry->d_name, MAXPATHLEN);

			struct stat entry_stat;

			if ((read_links ? stat : lstat)(full_path, &entry_stat)) {
				perror("stat");
				exit(EXIT_FAILURE);
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
					char *escaped = escape_string(full_path);
					printf("%s\n", escaped);
					free(escaped);
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
		.count = 0,
		.invert = 0,
		.limit = 0,
		.substring = 0,
		.wholename = 0,
	};

	int exit_code = EXIT_SUCCESS;
	int ch;
	char *remainder;
	size_t len = 0;
	const char *root = ".";
	FILE *fp;
	char cwd[MAXPATHLEN];
	int unmatched_error = 0;
	char wildcard = 0;
	char *override_config_file = NULL;

	if (getcwd(cwd, MAXPATHLEN) == NULL) {
		perror("getcwd");
		exit(EXIT_FAILURE);
	}

	char *xdg_config_home = getenv("XDG_CONFIG_HOME");
	char *home = getenv("HOME");

	while ((ch = getopt(argc, argv, "c:d:l:svw")) > -1) {
		switch (ch) {
		case 'c':
			override_config_file = optarg;
			break;

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

	char config_file[xdg_config_home
						 ? (strlen(xdg_config_home) + strlen("ignore") + 3)
						 : (strlen(cwd) + strlen(".rfignore") + 1)];

	if (xdg_config_home) {
		const char *pattern = "%s/rf/%s";
		int len = (strlen(pattern) + strlen(xdg_config_home) + strlen(CONFIG));
		snprintf(config_file, len, pattern, xdg_config_home, CONFIG);
	} else {
		const char *pattern = "%s/.config/rf/%s";
		int len = (strlen(pattern) + strlen(home) + strlen(CONFIG));
		snprintf(config_file, len, pattern, home, CONFIG);
	}

	fp = fopen(override_config_file ? override_config_file : config_file, "r");

	if (fp != NULL) {
		while ((config_get(&len, fp)) != -1) {
			if (strlen(config_key) && strlen(config_value)) {
				if (strcmp(config_key, "symlinks") == 0) {
					if (strcmp(config_value, "true") == 0) {
						read_links = 1;
					} else if (strcmp(config_value, "false") == 0) {
						read_links = 0;
					} else {
						fprintf(stderr,
							"'%s' is not a valid value for property: %s.\n",
							config_value, config_key);
						exit(EXIT_FAILURE);
					}
				} else if (strcmp(config_key, "wholename") == 0) {
					if (strcmp(config_value, "true") == 0) {
						switches.wholename = 1;
					} else if (strcmp(config_value, "false") == 0) {
						/* default */
					} else {
						fprintf(stderr,
							"'%s' is not a valid value for property: %s.\n",
							config_value, config_key);
						exit(EXIT_FAILURE);
					}
				} else if (strcmp(config_key, "limit") == 0) {
					int limit = strtol(config_value, &remainder, 10);

					if (limit < 0) {
						fprintf(stderr, "Warning: Invalid limit, ignoring.");
					} else {
						switches.limit = limit;
					}
				} else if (strcmp(config_key, "unmatched error") == 0) {
					unmatched_error = 1;
				} else if (strcmp(config_key, "wildcard") == 0) {
					wildcard = config_value[0];
				}
			} else if (strlen(config_key)) {
				fprintf(stderr,
					"Warning: Ignoring empty config property '%s'.\n",
					config_key);
			}
		}

		fclose(fp);
	} else if (override_config_file) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char global_ignore_path[(strlen(home) + strlen(".rfignore") + 1)];
	char config_ignore_path[xdg_config_home
								? (strlen(xdg_config_home) + strlen("ignore") +
									  3)
								: (strlen(cwd) + strlen(".rfignore") + 1)];
	char local_ignore_path[strlen(cwd) + strlen(".rfignore") + 1];

	const char *pattern = "%s/%s";
	snprintf(global_ignore_path,
		(strlen(pattern) + strlen(home) + strlen(RFIGNORE)), pattern, home,
		RFIGNORE);
	snprintf(local_ignore_path,
		(strlen(pattern) + strlen(cwd) + strlen(RFIGNORE)), pattern, cwd,
		RFIGNORE);

	if (xdg_config_home) {
		const char *pattern = "%s/rf/%s";
		int len = (strlen(pattern) + strlen(xdg_config_home) + strlen(IGNORE));
		snprintf(config_ignore_path, len, pattern, xdg_config_home, IGNORE);
	} else {
		const char *pattern = "%s/.config/rf/%s";
		int len = (strlen(pattern) + strlen(home) + strlen(IGNORE));
		snprintf(config_ignore_path, len, pattern, home, IGNORE);
	}

	global_ignores = init_ignores(global_ignore_path);
	config_ignores = init_ignores(config_ignore_path);
	local_ignores = init_ignores(local_ignore_path);

	if (optind < argc) {
		int i = 0;
		int pattern_count = argc - optind;
		char **patterns = (char **)calloc(sizeof(char *), pattern_count);

		while (optind < argc) {
			patterns[i++] = argv[optind++];

			if (wildcard) {
				for (size_t j = 0; j < strlen(patterns[i - 1]); j++) {
					if (patterns[i - 1][j] == wildcard) {
						patterns[i - 1][j] = '*';
					}
				}
			}
		}

		if (recurse_find(patterns, &pattern_count, root, &switches)) {
			/* finished early because we reached the limit */
		};

		free(patterns);

		if (unmatched_error && switches.count == 0) {
			exit_code = EXIT_FAILURE;
			goto bail;
		}
	} else {
		usage(NULL);
	}

bail:
	free_ignores(global_ignores);
	free_ignores(config_ignores);
	free_ignores(local_ignores);

	exit(exit_code);
}

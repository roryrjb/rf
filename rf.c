#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"

struct switches {
	int basename;
	int dirname;
	int invert;
	int limit;
	int count;
};

static int version(char *error)
{
	fprintf(stderr, "%s version %s\n", NAME, VERSION);
	return error == NULL ? 0 : 1;
}

static int usage(char *error)
{
	if (error != NULL) {
		fprintf(stderr, "Error: %s\n\n", error);
	}

	fprintf(stderr, "Usage: %s [OPTIONS]\n", NAME);
	fprintf(stderr, "\nOptions:\n");
	fprintf(stderr, "  --basename, -b   only show basename in results\n");
	fprintf(stderr, "  --dirname, -d    only show dirname in results\n");
	fprintf(stderr, "  --invert, -v     invert matching\n");
	fprintf(stderr,
			"  --limit=n        limit to [n] results, all if 0 [default=0]\n");

	fprintf(stderr, "\n  --help, -h       show help\n");
	fprintf(stderr, "  --version, -V    show version\n");
	fprintf(stderr, "\n");

	return version(error);
}

static int is_child(char *dirname)
{
	if (strcmp("..", dirname) == 0 || strcmp(".", dirname) == 0) {
		return 0;
	}

	return 1;
}

static int not_in_array(char **arr, char *dirname, size_t size)
{
	int i = 0;

	for (; i < size; i++) {
		if (strstr(dirname, arr[i])) {
			return 0;
		}
	}

	return 1;
}

static int at_side(int beginning, char *filename, char *str)
{
	int c = 0;
	int matched = 1;

	if (beginning) {
		for (; c < strlen(str); c++) {
			if (str[c] != filename[c]) {
				matched = 0;
				break;
			}
		}
	} else {
		int d = 0;

		for (c = strlen(str), d = strlen(filename); c >= 0; c--, d--) {
			if (str[c] != filename[d]) {
				matched = 0;
				break;
			}
		}
	}

	return matched;
}

static int excluded_extension(char *filename)
{
	int i = 0;

	for (; i < ignored_extensions_size; i++) {
		int res = at_side(0, filename, ignored_extensions[i]);

		if (res) {
			return res;
		}
	}

	return 0;
}

static void print_result(char *path, struct switches *switches,
						 struct dirent *entry)
{
	if (switches->basename) {
		printf("%s\n", entry->d_name);
	} else {
		char full_path[MAXPATHLEN];
		full_path[0] = '\0';
		strcat(full_path, path);

		if (!switches->dirname) {
			strcat(full_path, "/");
			strcat(full_path, entry->d_name);
		}

		printf("%s\n", full_path);
	}
}

static char *strslice(char *source, int start, int end)
{
	int i = 0;
	int diff = start + end;
	int len = strlen(source) - diff;
	char *output = malloc(sizeof(char *) * len);
	memset(output, '\0', len);

	for (; i < len; i++) {
		output[i] = source[i + start];
	}

	return output;
}

/* return 1 if breaking early (e.g. reaching limit) otherwise return 0 */
static int recurse_find(char **patterns, int *pattern_count, char *dirname,
						struct switches *switches)
{
	DIR *dir;

	char path[MAXPATHLEN] = { '\0' };
	int break_early = 0;
	strcat(path, dirname);
	dir = opendir(path);

	if (dir != NULL && not_in_array(ignored_dirs, dirname, ignored_dirs_size)) {
		struct dirent *entry;

		while ((entry = readdir(dir)) != NULL) {
			int matched = 0;
			int p = 0;

			switch (entry->d_type) {
			case DT_DIR:
				if (is_child(entry->d_name) &&
					not_in_array(ignored_dirs, entry->d_name,
								 ignored_dirs_size)) {
					char child_path[MAXPATHLEN] = { '\0' };
					strcat(child_path, path);
					strcat(child_path, "/");
					strcat(child_path, entry->d_name);
					if (recurse_find(patterns, pattern_count, child_path,
									 switches)) {
						break_early = 1;
						break;
					};
				}

				break;
			case DT_REG:
				if (excluded_extension(entry->d_name)) {
					break;
				}

				for (; p < *pattern_count; p++) {
					char *pattern = patterns[p];
					char first = pattern[0];
					char last = pattern[strlen(pattern) - 1];
					char *parsed = NULL;
					int arg_len = strlen(pattern);

					if (last == '$' && first == '^') {
						/* show everything */
						if (strlen(pattern) == 2) {
							matched = 1;
						} else {
							/* match whole string */
							parsed = strslice(pattern, 1, 1);

							if (strcmp(entry->d_name, parsed) == 0) {
								matched = 1;
							}
						}
					} else if (last == '$') {
						/* match at end */
						parsed = arg_len == 1 ? NULL : strslice(pattern, 0, 1);

						if (at_side(0, entry->d_name,
									arg_len == 1 ? pattern : parsed)) {
							matched = 1;
						}
					} else if (first == '^') {
						/* match at beginning */
						parsed = arg_len == 1 ? NULL : strslice(pattern, 1, 0);

						if (at_side(1, entry->d_name,
									arg_len == 1 ? pattern : parsed)) {
							matched = 1;
						}
					} else {
						/* substring match */
						if (strstr(entry->d_name, pattern) != NULL) {
							matched = 1;
						}
					}

					if (parsed != NULL) {
						free(parsed);
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

			if ((matched && (switches->invert == 0)) ||
				(matched == 0 && (switches->invert == 1))) {
				print_result(path, switches, entry);

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

int main(int argc, char **argv)
{
	static struct option options[] = { { "basename", no_argument, 0, 0 },
									   { "dirname", no_argument, 0, 0 },
									   { "invert", no_argument, 0, 0 },
									   { "limit", required_argument, 0, 0 },
									   { "version", no_argument, 0, 0 },
									   { "help", no_argument, 0, 0 },
									   { 0, 0, 0, 0 } };

	int basename = 0;
	int dirname = 0;
	int invert = 0;
	int limit = 0;
	int index = 0;
	int res;
	int count = 0; /* used to count how matches we find */

	char *remainder;

	while ((res = getopt_long(argc, argv, "hvVbd", options, &index)) > -1) {
		switch (res) {
		case 0:
			if (strcmp("version", options[index].name) == 0) {
				return version(NULL);
			} else if (strcmp("help", options[index].name) == 0) {
				return usage(NULL);
			} else if (strcmp("basename", options[index].name) == 0) {
				basename = 1;
			} else if (strcmp("dirname", options[index].name) == 0) {
				dirname = 1;
			} else if (strcmp("invert", options[index].name) == 0) {
				invert = 1;
			} else if (strcmp("limit", options[index].name) == 0) {
				limit = strtol(optarg, &remainder, 10);

				if (limit < 0) {
					return usage("Invalid limit.");
				}
			}

			break;

		case 'V':
			return version(NULL);

		case 'h':
			return usage(NULL);

		case 'b':
			basename = 1;
			break;

		case 'd':
			dirname = 1;
			break;

		case 'v':
			invert = 1;
			break;
		}
	}

	if (optind < argc) {
		while (optind < argc) {
			optind++;
		}
	}

	if (optind > 1) {
		int i = 0;
		struct switches switches;
		int pattern_count = optind - 1;
		char **patterns = malloc(sizeof(char *) * optind);

		memset(patterns, '\0', optind);

		for (; i < optind; i++) {
			patterns[i] = argv[i + 1];
		}

		switches.basename = basename;
		switches.dirname = dirname;
		switches.invert = invert;
		switches.limit = limit;
		switches.count = count;

		if (recurse_find(patterns, &pattern_count, ".", &switches)) {
			/* finished early because we reached the limit */
		};

		free(patterns);
	}

	return 0;
}

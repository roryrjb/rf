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

#define NAME "rf"
#define VERSION "0.0.1"

struct switches {
  int basename;
  int invert;
  int limit;
  int count;
};

int version(char *error) {
  fprintf(stderr, "%s version %s\n", NAME, VERSION);
  return error == NULL ? 0 : 1;
}

int usage(char *error) {
  if (error != NULL) {
    fprintf(stderr, "Error: %s\n\n", error);
  }

  fprintf(stderr, "Usage: %s [OPTIONS]\n", NAME);
  fprintf(stderr, "\nOptions:\n");
  fprintf(stderr, "  --basename, -b   only show basename in results\n");
  fprintf(stderr, "  --invert, -v     invert matching\n");
  fprintf(stderr,
          "  --limit=n        limit to [n] results, all if 0 [default=0]\n");

  fprintf(stderr, "\n  --help, -h       show help\n");
  fprintf(stderr, "  --version, -V    show version\n");
  fprintf(stderr, "\n");

  return version(error);
}

int is_child(char *dirname) {
  if (strcmp("..", dirname) == 0 || strcmp(".", dirname) == 0) {
    return 0;
  }

  return 1;
}

int not_in_array(char **arr, char *dirname, size_t size) {
  for (int i = 0; i < size; i++) {
    if (strstr(dirname, arr[i])) {
      return 0;
    }
  }

  return 1;
}

int at_side(int beginning, char *filename, char *str) {
  int matched = 1;

  if (beginning) {
    for (int c = 0; c < strlen(str); c++) {
      if (str[c] != filename[c]) {
        matched = 0;
        break;
      }
    }
  } else {
    for (int c = strlen(str), d = strlen(filename); c >= 0; c--, d--) {
      if (str[c] != filename[d]) {
        matched = 0;
        break;
      }
    }
  }

  return matched;
}

int excluded_extension(char *filename) {
  for (int i = 0; i < ignored_extensions_size; i++) {
    int res = at_side(0, filename, ignored_extensions[i]);

    if (res) {
      return res;
    }
  }

  return 0;
}

void print_result(char *path, struct switches *switches, struct dirent *entry) {
  if (switches->basename) {
    printf("%s\n", entry->d_name);
  } else {
    char full_path[MAXPATHLEN];
    full_path[0] = '\0';
    strcat(full_path, path);
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);
    printf("%s\n", full_path);
  }
}

char *strslice(char *source, int start, int end) {
  int diff = start + end;
  int len = strlen(source) - diff;
  char *output = malloc(sizeof(char *) * len);
  memset(output, '\0', len);

  for (int i = 0; i < len; i++) {
    output[i] = source[i + start];
  }

  return output;
}

void recurse_find(char **patterns, int *pattern_count, char *dirname,
                  struct switches *switches) {
  char path[PATH_MAX] = {'\0'};
  strcat(path, dirname);

  DIR *dir = opendir(path);

  if (dir != NULL && not_in_array(ignored_dirs, dirname, ignored_dirs_size)) {
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
      int matched = 0;

      switch (entry->d_type) {
      case DT_DIR:
        if (is_child(entry->d_name) &&
            not_in_array(ignored_dirs, entry->d_name, ignored_dirs_size)) {
          char child_path[PATH_MAX] = {'\0'};
          strcat(child_path, path);
          strcat(child_path, "/");
          strcat(child_path, entry->d_name);
          recurse_find(patterns, pattern_count, child_path, switches);
        }

        break;
      case DT_REG:
        if (excluded_extension(entry->d_name)) {
          break;
        }

        for (int p = 0; p < *pattern_count; p++) {
          char *pattern = patterns[p];
          char first = pattern[0];
          char last = pattern[strlen(pattern) - 1];
          char *parsed = NULL;

          if (last == '$' && first == '^') {
            // show everything
            if (strlen(pattern) == 2) {
              matched = 1;
            } else {
              // match whole string
              parsed = strslice(pattern, 1, 1);

              if (strcmp(entry->d_name, parsed) == 0) {
                matched = 1;
              }
            }
          } else if (last == '$') {
            // match at end
            parsed = strslice(pattern, 0, 1);

            if (at_side(0, entry->d_name, parsed)) {
              matched = 1;
            }
          } else if (first == '^') {
            // match at beginning
            parsed = strslice(pattern, 1, 0);

            if (at_side(1, entry->d_name, parsed)) {
              matched = 1;
            }
          } else {
            // substring match
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
      }

      if ((matched && (switches->invert == 0)) ||
          (matched == 0 && (switches->invert == 1))) {
        print_result(path, switches, entry);

        if (switches->limit > 0 && ++switches->count == switches->limit) {
          exit(EXIT_SUCCESS);
        }
      }
    }

    closedir(dir);
  }
}

int main(int argc, char **argv) {
  static struct option options[] = {
      {"basename", no_argument, 0, 0},    {"invert", no_argument, 0, 0},
      {"limit", required_argument, 0, 0}, {"version", no_argument, 0, 0},
      {"help", no_argument, 0, 0},        {0, 0, 0, 0}};

  int basename = 0;
  int invert = 0;
  int limit = 0;

  int index = 0;
  int res;

  char *remainder;

  while ((res = getopt_long(argc, argv, "hvVb", options, &index)) > -1) {
    switch (res) {
    case 0:
      if (strcmp("version", options[index].name) == 0) {
        return version(NULL);
      } else if (strcmp("help", options[index].name) == 0) {
        return usage(NULL);
      } else if (strcmp("basename", options[index].name) == 0) {
        basename = 1;
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

    case 'v':
      invert = 1;
      break;
    }
  }

  if (optind < argc) {
    while (optind < argc) {
      optind++;
    }

    int pattern_count = optind - 1;

    char **patterns = malloc(sizeof(char *) * optind);
    memset(patterns, '\0', optind);

    for (int i = 0; i < optind; i++) {
      patterns[i] = argv[i + 1];
    }

    int count = 0; // used to count how matches we find

    struct switches switches = {
        .basename = basename, .invert = invert, .limit = limit, .count = count};

    recurse_find(patterns, &pattern_count, ".", &switches);
    free(patterns);
  }

  return 0;
}

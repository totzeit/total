/**
 * -*- coding: utf-8; -*-
 *
 * @file total/total.c
 *
 * Copyright © 2008-2013 Totzeit, Inc.
 *
 * This work is licensed under
 *
 *     Creative Commons Attribution 3.0 Unported License (CC BY 3.0)
 *
 * the full text of which may be retrieved at
 *
 *     http://creativecommons.org/licenses/by/3.0/
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#define MAX_FIELDS   (CHAR_BIT*sizeof (uint64_t))
#define MAX_FILES    2048

typedef union {
    int64_t  i;
    double   f;
} value_t;

char     *program          = NULL;
int       quiet            = false;
int       totals_only      = false;
bool      horizontal       = false;
bool      doubles          = false;

char     *separator        = ",";
char     *output_separator = NULL;

uint64_t  include          = 0xFFFFFFFFFFFFFFFFULL;

value_t   file_totals[MAX_FIELDS];
value_t   totals[MAX_FIELDS];

char     *fields[MAX_FIELDS];
int       last_field_seen  = 0;

int       filename_count   = 0;
char     *filenames[MAX_FILES];

int       separator_length = 1;
bool      show_header      = false;

int split(char *line) {
    char *start = line;
    char *sep = NULL;
    int   index = 0;

    if (separator_length == 1) {
        while (*start && (sep = strchr(start, *separator)) != NULL) {
            *sep = '\0';
            fields[index++] = start;
            start = sep + 1;
        }
    } else {
        while (*start && (sep = strstr(start, separator)) != NULL) {
            *sep = '\0';
            fields[index++] = start;
            start = sep + separator_length - 1;
        }
    }

    if (start && *start) {
        fields[index++] = start;
    }

    return index;
}

#define SETFIELD(x) (include |= (1ULL << (x))) 
#if 0
{ unsigned __x = (x); printf("Setting field %d: 0x%016llux\n", __x, (1ULL << __x)); include |= (1ULL << (x)); }
#endif /* 0 */
#define GETFIELD(x) ((include&(1ULL << (x))) > 0 ? 1 : 0)

void include_range(int start, int end) {
    /* fprintf(stderr, "Including range [%d,%d]\n", start, end); */
    if (start < 0) start = 0;
    if (start <= end) {
        while (start <= end) {
            SETFIELD(start);
            start++;
        }
    } else {
        while (end <= start) {
            SETFIELD(end);
            end++;
        }
    }
}

void usage_and_exit(int code) {
    fprintf(stderr, "Usage %s [OPTIONS] [FILE]\n", program);
    fprintf(stderr, 
            "Where OPTIONS includes:\n"
            "\n"
            "-s | --separator <sep>            Assume input fields separated by <sep>\n"
            "-S | --output-separator <sep>     Output separated by <sep>\n"
            "-f | --field <num>[,<num>]        Total field(s) <num>[,<num>]\n"
            "-t | --totals-only                Only print grant totals\n"
            "-z | --horizontal                 Total lines horizontally\n"
            "-d | --doubles                    Assume values are floating point\n"
            "-q | --quiet                      Ignore errors\n"
            "-H | --header                     Print column headers\n"
            "-h | --help                       You're soaking in it\n"
            "-v | --version                    Probably idempotent\n"
            "\n"
            "Defaults are:\n"
            "\n"
            "  -f 1- -s \"%s\"\n"
            "\n"
            "with the output separator the same as the input separator.\n"
            "\n"
            "If FILE is left off, stdin is read. Note that in the presence of -z,\n"
            "options -t and -H are ignored.\n",
            separator);
            
    exit(code);
}

int long_option = 0;
struct option opts[] = {
    { "help",              0, &long_option, 'h' },
    { "version",           0, &long_option, 'v' },
    { "separator",         1, &long_option, 's' },
    { "output-separator",  1, &long_option, 'S' },
    { "field",             1, &long_option, 'f' },
    { "totals",            0, &long_option, 't' },
    { "horizontal",        0, &long_option, 'z' },
    { "doubles",           0, &long_option, 'd' },
    { "header",            0, &long_option, 'H' },
    { "quiet",             0, &long_option, 'q' },
};
#define OPTS "hvs:S:f:qtzdH"

void parse_fields(char *list) {
    char *start = list;
    char *end   = NULL;

    long start_index = -1;
    bool in_range    = false;
 
    for (;;) {
        switch (*start) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                long n = strtol(start, &end, 10);
                if (n > 0) {
                    n--;
                } else {
                    n = 0;
                }

                if (in_range) {
                    include_range(start_index, n);
                    in_range = false;
                    start_index = -1;
                } else {
                    start_index = n;
                }
                start = end;
            }
            break;

            case '-':
                start++;
                in_range = true;
                break;

            case '\0':
            case ',':
                if (in_range) {
                    include_range(start_index, MAX_FIELDS - 1);
                } else {
                    if (start_index >= 0) SETFIELD(start_index);
                }
                start_index = -1;
                in_range = false;
                if (*start == '\0') return;
                start++;
                break;
            default:
                usage_and_exit(1);
        }
    }
}

void version(void) {
    printf("%s %d.%d\n", program, VERSION_MAJOR, VERSION_MINOR);
    exit(EXIT_SUCCESS);
}

void parse_opts(int argc, char *argv[]) {
    int opt_ind = 0;
    int c;
    while ((c = getopt_long(argc, argv, OPTS, opts, &opt_ind)) != -1) {
        if (c == 0) c = long_option;
        switch (c) {
            case 'h':
                usage_and_exit(EXIT_SUCCESS);
            case 'v':
                version();
            case 'f':
                include = 0ULL;
                parse_fields(optarg);
                break;
            case 's':
                separator = strdup(optarg);
                separator_length = strlen(separator);
                break;
            case 'S':
                output_separator = strdup(optarg);
                break;
            case 't':
                totals_only = true;
                break;
            case 'z':
                horizontal = true;
                break;
            case 'd':
                doubles = true;
                break;
            case 'H':
                show_header = true;
                break;
            case 'q':
                quiet = true;
                break;
            case '?':
            default:
                usage_and_exit(EXIT_FAILURE);
        }
    }

    output_separator = output_separator ? output_separator : separator;

    while (optind < argc) {
        if (*argv[optind] != '-') {
            filenames[filename_count++] = strdup(argv[optind]);
        }
        optind++;
    }
}

void total_line(char *line) {
    int fcount = split(line);
    int i, index = 0;

    last_field_seen = (last_field_seen > fcount ? last_field_seen : fcount);

    if (horizontal) {
        if (doubles) {
            totals[0].f = 0.0;
        } else {
            totals[0].i = 0;
        }
    }

    for (i = 0; i < fcount; i++) {
        if (GETFIELD(i) > 0) {
            if (doubles) {
                double v = strtod(fields[i], NULL);
                if (!horizontal) {
                    file_totals[index].f += v;
                } else {
                    totals[0].f += v;
                }
            } else {
                int64_t v = strtol(fields[i], NULL, 0);
                if (!horizontal) {
                    file_totals[index].i += v;
                } else {
                    totals[0].i += v;
                }
            }
            index++;
        }
    }

    if (horizontal) {
        if (doubles) {
            printf("%lf\n", totals[0].f);
        } else {
            printf("%ld\n", totals[0].i);
        }
    }
}

void print_totals(char *header, value_t tots[]) {
    int i, index = 0;
    if (show_header && header) {
        printf("%s: ", header);
    }

    for (i = 0; i < last_field_seen; i++) {
        if (GETFIELD(i) > 0) {
            if (doubles) {
                printf("%s%lf", (index > 0 ? output_separator : ""), tots[index].f);
            } else {
                printf("%s%ld", (index > 0 ? output_separator : ""), tots[index].i);
            }
            index++;
        }
    }

    if (last_field_seen > 0) {
        printf("\n");
    }
}

int total_file(char *fname, FILE *in) {
    char    *line = NULL;
    size_t   len;
    ssize_t  read;

    if (doubles) {
        memset(file_totals, 0, MAX_FIELDS*sizeof (double));
    } else {
        memset(file_totals, 0, MAX_FIELDS*sizeof (int64_t));
    }

    while ((read = getline(&line, &len, in)) != -1) {
        if (read > 0) {
            line[read - 1] = '\0';
        }

        total_line(line);
    }

    if (line) free(line);

    if (!totals_only && !horizontal) {
        print_totals(fname, file_totals);
    }

    return 0;
}

int total(void) {
    int i;

    if (filename_count == 0) {
        /* stdin */
        total_file("<stdin>", stdin);
    } else {
        for (i = 0; i < filename_count; i++) {

            FILE *in = fopen(filenames[i], "rm");

            if (in != NULL) {
                total_file(filenames[i], in);
            } else {
                if (!quiet) {
                    fprintf(stderr, "Unable to open file `%s': %s\n", filenames[i], strerror(errno));
                }
                return 1;
            }
        }
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    program = basename(argv[0]);
    parse_opts(argc, argv);

    int err = 0;

    if ((err = total()) != 0) {
        return err;
    }

    if (filename_count > 1 && !horizontal) {
        print_totals("Totals", totals);
    }

    return 0;
}

/**
 * Local Variables:
 * indent-tabs-mode: nil
 * fill-column: 79
 * comment-column: 37
 * End:
 */

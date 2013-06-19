/*
 * total.c
 *
 * Total a field (column) in a file.
 *
 * Copyright (c) 2000 Chris Kirkwood-Watts <kirkwood@acm.org>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#define MAX_FIELDS   (CHAR_BIT*sizeof (uint64_t))
#define MAX_FILES    2048

char		*program	  = NULL;
int		 quiet		  = 0;
int		 totals_only	  = 0;

char		*separator        = ",";

uint64_t	 include	  = 0xFFFFFFFFFFFFFFFFULL;

long  file_totals[MAX_FIELDS]	  = { 0 };
long  totals[MAX_FIELDS]	  = { 0 };

char *fields[MAX_FIELDS]	  = { NULL };
int		 last_field_seen  = 0;

int		 filename_count	  = 0;
char *filenames[MAX_FILES]	  = { NULL };

int		 separator_length = 1;
int		 header		  = 0;

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
            "-s | --separator <separator>      Fields separated by <separator>\n"
            "-f | --field <num>[,<num>]        Total field(s) <num>[,<num>]\n"
            "-t | --totals-only                Only print grant totals\n"
            "-q | --quiet                      Ignore errors\n"
            "-h | --help                       You're soaking in it\n"
            "-v | --version                    Probably idempotent\n"
            "\n"
            "Defaults are:\n"
            "\n"
            "  -f 1- -s \"%s\"\n"
            "\n"
            "If FILE is left off, stdin is read.\n",
            separator);
            
    exit(code);
}

int long_option = 0;
struct option opts[] = {
    { "help",          0, &long_option, 'h' },
    { "version",       0, &long_option, 'v' },
    { "separator",     1, &long_option, 's' },
    { "field",         1, &long_option, 'f' },
    { "totals",        0, &long_option, 't' },
    { "header",        0, &long_option, 'H' },
    { "quiet",         0, &long_option, 'q' },
};
#define OPTS "hvs:f:qtH"

void parse_fields(char *list) {
    char *start = list;
    char *end   = NULL;

    long start_index = -1;
    int  in_range    = 0;
 
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
                    in_range = 0;
                    start_index = -1;
                } else {
                    start_index = n;
                }
                start = end;
            }
            break;

            case '-':
                start++;
                in_range = 1;
                break;

            case '\0':
            case ',':
                if (in_range) {
                    include_range(start_index, MAX_FIELDS - 1);
                } else {
                    if (start_index >= 0) SETFIELD(start_index);
                }
                start_index = -1;
                in_range = 0;
                if (*start == '\0') return;
                start++;
                break;
            default:
                usage_and_exit(1);
        }
    };

#if 0
 parse_out:

    int i, count = 0;
    fprintf(stderr, "Fields:");
    for (i = 0; i < MAX_FIELDS; i++) {
        if (GETFIELD(i)) {
            fprintf(stderr, "%s%d", ((count > 0) ? ", " : " "), i + 1);
            count++;
        }
    }
    fprintf(stderr, "\n");
#endif /* 0 */
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
                /* printf("Fields: 0x%016lux\n", include); */
                break;
            case 's':
                separator = strdup(optarg);
                separator_length = strlen(separator);
                break;
            case 't':
                totals_only = 1;
                break;
            case 'H':
                header = 1;
                break;
            case 'q':
                quiet = 1;
                break;
            case '?':
            default:
                usage_and_exit(EXIT_FAILURE);
        }
    }

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

    for (i = 0; i < fcount; i++) {
        if (GETFIELD(i) > 0) {
            long val = atol(fields[i]);
            /* printf("%d += %ld\n", index, val); */
            file_totals[index] += val;
            totals[index] += val;
            
            index++;
        }
    }
}

void print_totals(char *header, long tots[]) {
    int i, index = 0;
    if (header && header) {
        printf("%s: ", header);
    }

    for (i = 0; i < last_field_seen; i++) {
        if (GETFIELD(i) > 0) {
            printf("%s%ld", (index > 0 ? " " : ""), tots[index]);
            index++;
        }
    }
    if (last_field_seen > 0) printf("\n");
}

int total_file(char *fname, FILE *in) {
    char    *line = NULL;
    size_t   len;
    ssize_t  read;

    memset(file_totals, 0, MAX_FIELDS*sizeof (long));

    while ((read = getline(&line, &len, in)) != -1) {
        if (read > 0) {
            line[read - 1] = '\0';
        }

        total_line(line);
    }

    if (line) free(line);

    if (!totals_only) {
        print_totals(fname, file_totals);
    }

    return 0;
}

int total(void) {
    int i;
    memset(totals, 0, MAX_FIELDS*sizeof (long));
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

    if (filename_count > 1) print_totals("Totals", totals);
    return 0;
}


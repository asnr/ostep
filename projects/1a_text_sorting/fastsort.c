#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

typedef struct options_type {
    char *filename;
    long word_to_compare;
} options_t;

typedef struct inputline {
    char *start;
    char *sortfrom;
} inputline_t;


const int MAX_LINE_LEN = 128;
const int LINE_BUF_SIZE = MAX_LINE_LEN + 1;
const char DELIM[] = " ";

void bad_args_print_and_exit();
void exit_if_malloc_failed(char *p);
void exit_if_could_not_open_file(FILE *p, const char *filename);

options_t parse_command_line_options(int argc, char const *argv[]);

void print_all_values(char *values, char* values_bookend);

char *find_key(char *line, int word_to_compare);

int main(int argc, char const *argv[])
{
    options_t options = parse_command_line_options(argc, argv);

    long word_to_compare = options.word_to_compare;
    char *filename = options.filename;

    // You should check the arguments of fastsort carefully.
    // If more than two arguments are passed, or two are passed but
    // the second does not fit the format of a dash followed by a number,
    // you should EXACTLY print (to standard error): Error: Bad command line parameters and exit with return code 1.

    struct stat file_stats;
    int statrc = stat(filename, &file_stats);
    if (statrc == -1) {
        printf("Error: stat() failed.\n");
        exit(1);
    }

    long long file_bytes = file_stats.st_size;

    char *values = (char *) malloc(file_bytes + 1);
    exit_if_malloc_failed(values); // TODO: test this! Artificially limit memory available to process?
    
    char *keys = (char *) malloc(file_bytes + 1);
    exit_if_malloc_failed(keys);
    
    FILE *fp = fopen(filename, "r");
    exit_if_could_not_open_file(fp, filename);
    
    char *fst_value_char = values;
    char *fst_key_char = keys;
    char *lst_value_char;
    while (fgets(fst_value_char, MAX_LINE_LEN, fp) != NULL) {
        // Note when fgets succeeds, it always appends '\0' to the string
        for (lst_value_char = fst_value_char;
             *lst_value_char != '\0' && *lst_value_char != '\n';
             lst_value_char++);

        if (*lst_value_char == '\n') {
            *lst_value_char = '\0';
        } else if (!feof(fp)) {
            fprintf(stderr, "Line too long\n");
            exit(1);
        }

        memcpy(fst_key_char, fst_value_char,
               lst_value_char - fst_value_char + 1);

        char *key = find_key(fst_key_char, word_to_compare);

        printf("key: \"%s\", line: \"%s\"\n", key, fst_value_char);

        fst_value_char = lst_value_char + 1;
    }
    // char *values_bookend = fst_value_char;

    // print_all_values(values, values_bookend);

    fclose(fp);

    free(keys);
    free(values);

    return 0;
}


void bad_args_print_and_exit()
{
    printf("Error: Bad command line parameters\n");
    exit(1);
}


void exit_if_malloc_failed(char *p)
{
    if (p == NULL) {  // TODO: test this! Artificially limit memory available to process?
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
}


void exit_if_could_not_open_file(FILE *p, const char *filename)
{
    if (p == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        exit(1);
    }
}


options_t parse_command_line_options(int argc, char const *argv[])
{
    options_t options;

    if (argc == 2) {
        options.word_to_compare = 0;
        options.filename = (char *) argv[1];
    } else if (argc == 3) {
        options.filename = (char *) argv[2];
        if (*argv[1] != '-') {
            bad_args_print_and_exit();
        }
        char *end_char;
        long word_arg = strtol(argv[1] + 1, &end_char, 10);
        if ((word_arg == 0 && errno == EINVAL) || *end_char != '\0') {
            bad_args_print_and_exit();
        }
        options.word_to_compare = word_arg - 1;
    } else {
        bad_args_print_and_exit();
    }

    return options;
}


void print_all_values(char *values, char* values_bookend)
{
    for (char *curr = values; curr < values_bookend; curr++) {
        if (*curr == '\0') {
            putchar('\n');
        } else {
            putchar(*curr);
        }
    }
}


char *find_key(char *line, int word_to_compare)
{
    if (*line == '\0') {
        return line;
    }

    int word_idx = 0;
    char *next_word = line;
    char *key = line;
    char *curr_word;
    while (word_idx <= word_to_compare) {
        curr_word = strsep(&next_word, DELIM);
        if (*curr_word != '\0') {
            word_idx++;
            key = curr_word;
        }
        if (next_word == NULL) {
            break;
        }
    }

    return key;
}

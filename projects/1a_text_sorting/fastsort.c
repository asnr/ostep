// ./fastsort
// ./fastsort a b c
// ./fastsort a b
// ./fastsort -1a
// ./fastsort -1a f
// ./fastsort 12 f
// ./fastsort 12 s
// ./fastsort -1 invalid_input_1_too_long.txt
// ./fastsort -1 invalid_input_2_line_too_long_without_newline.txt
// ./fastsort -1 valid_input_1.txt
// ./fastsort -2 valid_input_1.txt
// ./fastsort -9999999 valid_input_1.txt
// ./fastsort -1 valid_input_2_no_last_new_line.txt
// ./fastsort -6 valid_input_2_no_last_new_line.txt
// ./fastsort -7 valid_input_2_no_last_new_line.txt
// ./fastsort -10000 valid_input_3_spaces_at_end_no_newline.txt
// ./fastsort -6 valid_input_4_spaces_at_end_with_newline.txt
// ./fastsort -2 valid_input_5_empty_file.txt  # this finds zero lines for now
// ./fastsort -1 valid_input_6_spaces_at_start_no_newline.txt  # key is 'starts'
// ./fastsort -1 valid_input_7_reverse_order.txt


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

typedef struct inputline_type {
    char *line;
    char *key;
} inputline_t;

typedef struct inputline_vec_type {
    inputline_t *inputlines;
    size_t size;
    size_t capacity;
} inputline_vec_t;


const int MAX_LINE_LEN = 128;
const int LINE_BUF_SIZE = MAX_LINE_LEN + 1;
const char DELIM[] = " ";

void bad_args_print_and_exit();
void exit_if_stat_failed(int statrc);
void exit_if_malloc_failed(void *p);
void exit_if_could_not_open_file(FILE *p, const char *filename);

options_t parse_command_line_options(int argc, char const *argv[]);

void read_input_file(char *filename, long word_to_compare,
                     char *values, char *keys, inputline_vec_t *inputline_vec);

void print_all_values(char *values, char* values_bookend);

char *find_key(char *line, int word_to_compare);

int compare_lines(const void *line_a, const void *line_b);

int main(int argc, char const *argv[])
{
    options_t options = parse_command_line_options(argc, argv);

    long word_to_compare = options.word_to_compare;
    char *filename = options.filename;

    struct stat file_stats;
    int statrc = stat(filename, &file_stats);
    exit_if_stat_failed(statrc);

    long long file_bytes = file_stats.st_size;

    char *values = (char *) malloc(file_bytes + 1);
    exit_if_malloc_failed(values); // TODO: test this! Artificially limit memory available to process?
    
    char *keys = (char *) malloc(file_bytes + 1);
    exit_if_malloc_failed(keys);

    inputline_vec_t inputlines;
    read_input_file(filename, word_to_compare, values, keys, &inputlines);

    qsort(inputlines.inputlines, inputlines.size, sizeof(inputline_t),
          &compare_lines);

    inputline_t *fst_line = inputlines.inputlines;
    for (int line_idx = 0; line_idx < inputlines.size; line_idx++) {
        inputline_t curr = *(fst_line + line_idx);
        printf("key: \"%s\", line: \"%s\"\n", curr.key, curr.line);
    }

    // print_all_values(values, values_bookend);

    free(inputlines.inputlines);
    free(keys);
    free(values);

    return 0;
}


void bad_args_print_and_exit()
{
    printf("Error: Bad command line parameters\n");
    exit(1);
}


void exit_if_stat_failed(int statrc)
{
    if (statrc == -1) {
        printf("Error: stat() failed.\n");
        exit(1);
    }
}


void exit_if_malloc_failed(void *p)
{
    if (p == NULL) {  // TODO: test this! Artificially limit memory available to process?
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
}


void exit_if_calloc_failed(void *p)
{
    if (p == NULL) {  // TODO: test this! Artificially limit memory available to process?
        fprintf(stderr, "calloc failed\n");
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
        // Defensive assignment
        options = (options_t) { .word_to_compare = 0, .filename = NULL };
        bad_args_print_and_exit();
    }

    return options;
}


void init_vec(inputline_vec_t *inputline_vec)
{
    size_t starting_capacity = 1;// 1048576;  // 2^20
    inputline_vec->size = 0;
    inputline_t *new_inputlines = calloc(starting_capacity,
                                          sizeof(inputline_t));
    exit_if_calloc_failed(new_inputlines);
    inputline_vec->inputlines = new_inputlines;
    inputline_vec->capacity = starting_capacity;
}


void double_vec_capacity(inputline_vec_t *inputline_vec) {
    size_t new_capacity = 2 * (inputline_vec->capacity);
    inputline_t *new_inputlines = calloc(new_capacity, sizeof(inputline_t));
    exit_if_calloc_failed(new_inputlines);

    memcpy(new_inputlines, inputline_vec->inputlines,
           (inputline_vec->size)*sizeof(inputline_t));

    free(inputline_vec->inputlines);
    inputline_vec->inputlines = new_inputlines;
    inputline_vec->capacity = new_capacity;
}


void append_inputline(inputline_vec_t *inputline_vec, char *line, char *key)
{
    if (inputline_vec->size == inputline_vec->capacity) {
        double_vec_capacity(inputline_vec);
    }

    inputline_t *new_inputline =
        (inputline_vec->inputlines) + (inputline_vec->size);
    new_inputline->line = line;
    new_inputline->key = key;
    inputline_vec->size += 1;
} 


void read_input_file(char *filename, long word_to_compare,
                     char *values, char *keys, inputline_vec_t *inputline_vec)
{
    FILE *fp = fopen(filename, "r");
    exit_if_could_not_open_file(fp, filename);

    init_vec(inputline_vec);

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

        int line_length = lst_value_char - fst_value_char;
        memcpy(fst_key_char, fst_value_char,
               (line_length + 1) * sizeof(*fst_key_char));

        char *key = find_key(fst_key_char, word_to_compare);

        append_inputline(inputline_vec, fst_value_char, key);

        fst_value_char += line_length + 1;
        fst_key_char += line_length + 1;
    }

    fclose(fp);
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


int compare_lines(const void *line_a, const void *line_b)
{
    inputline_t *a = (inputline_t*) line_a;
    inputline_t *b = (inputline_t*) line_b;
    return strcmp(a->key, b->key);
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

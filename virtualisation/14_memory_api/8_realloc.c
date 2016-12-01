/*
8. Try out some of the other interfaces to memory allocation. For
example, create a simple vector-like data structure and related
routines that use realloc() to manage the vector. Use an array to store
the vectors elements; when a user adds an entry to the vector, use
realloc() to allocate more space for it. How well does such a vector
perform? How does it compare to a linked list? Use valgrind to help you
find bugs.
*/

#include <stdlib.h>
#include <stdio.h>

static const char PROMPT[] =
    "Type number to add to vector, press enter to print > ";


typedef enum {NUM, PRINT, ERR} parseresult_t;

typedef int elm_t;

typedef struct {
    elm_t *elms;
    size_t size;
    size_t capacity;
} vec_t;

void init_vec(vec_t *vec)
{
    vec->elms = (elm_t *) malloc(1 * sizeof(*(vec->elms)));
    if (vec->elms == NULL) {
        printf("Error: malloc() failed\n");
        exit(1);
    }
    vec->size = 0;
    vec->capacity = 1;
}

void destroy_vec(vec_t *vec)
{
    free(vec->elms);
}

void append(vec_t *vec, int x)
{
    if (vec->capacity == vec->size) {
        printf("realloc'ing...\n");
        size_t new_mem_size = (vec->capacity + 1) * sizeof(*(vec->elms));
        elm_t *newptr = (elm_t *) reallocf(vec->elms, new_mem_size);
        if (newptr == NULL) {
            printf("Error: realloc() failed\n");
            exit(1);
        }
        if (newptr == vec->elms) {
            printf("Did not have to request new memory\n");
        }
        vec->elms = newptr;
        vec->capacity += 1;
    }

    vec->elms[vec->size] = x;
    vec->size += 1;
}

void printvec(vec_t *vec)
{
    for (int idx = 0; idx < vec->size; idx++) {
        printf("%d\n", vec->elms[idx]);
    }
}

parseresult_t parseinput(char *input, int *num)
{
    if (input[0] == '\0' || input[0] == '\n') {
        return PRINT;
    }

    char *endptr;
    *num = (int) strtol(input, &endptr, 10);

    if (*endptr == '\0' || *endptr == '\n') {
        return NUM;
    } else {
        return ERR;
    }
}

int main(int argc, char const *argv[])
{
    vec_t vec;
    init_vec(&vec);

    int nextnum;
    char input[21];

    for (;;) {
        printf("%s", PROMPT);

        if (fgets(input, 20, stdin) == NULL) {
            printf("\n");
            break;
        }

        parseresult_t rc = parseinput(input, &nextnum);

        switch (rc) {
            case NUM:
                append(&vec, nextnum);
                break;
            case PRINT:
                printf("Vector so far:\n");
                printvec(&vec);
                break;
            case ERR:
            default:
                printf("Error\n");
        }
    }

    destroy_vec(&vec);

    return 0;
}

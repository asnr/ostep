#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>


static const char ERROR_MSG[] = "An error has occurred\n";
static const char PROMPT[] = "whoosh> ";
static const char REDIRECT[] = ">";
static const char ARG_DELIM[] = " \t";  // command line delimiters
static const int MAX_LINE_LEN = 128;

#define RET_OK   0
#define RET_ERR  1

#define continue_on_error(errcode)  \
    if (errcode == RET_ERR) {       \
        print_error();              \
        continue;                   \
    }


typedef struct pathdirlist {
    char *pathdirs[MAX_LINE_LEN];
    int size;
} path_t;

void init_path(path_t *path);
void free_path(path_t *path);

void print_error()
{
    write(STDERR_FILENO, ERROR_MSG, strlen(ERROR_MSG));
}

void error_and_exit()
{
    print_error();
    exit(1);
}



void alloc_str_array(char *strs[], int str_len, int num_strs);
int read_input_line(char *buf, int max_line_len);
int tokenise(char *input, int max_line_len, char *tokens[]);
void runcmd(char *tokens[], int numtokens, path_t *path);
void free_str_array(char *strs[], int numstrs);

void print_tokens(char *tokens[], int numtokens)
{
    for (int idx = 0; idx < numtokens; idx++) {
        printf("\"%s\"\n", tokens[idx]);
    }
}

int main(int argc, char const *argv[])
{
    char input[MAX_LINE_LEN + 1];
    char *tokens[MAX_LINE_LEN];
    path_t path;

    alloc_str_array(tokens, MAX_LINE_LEN + 1, MAX_LINE_LEN);
    init_path(&path);

    for (;;) {

        printf("%s", PROMPT);

        int readrc = read_input_line(input, MAX_LINE_LEN);
        continue_on_error(readrc)

        int numtokens = tokenise(input, MAX_LINE_LEN, tokens);

        runcmd(tokens, numtokens, &path);

        // print_tokens(tokens, numtokens);
    }

    free_path(&path);
    free_str_array(tokens, MAX_LINE_LEN);

    return 0;
}

void init_path(path_t *path)
{
    alloc_str_array(path->pathdirs, MAX_LINE_LEN + 1, MAX_LINE_LEN);
    strcpy(path->pathdirs[0], "/bin");
    path->size = 1;
}

void free_path(path_t *path)
{
    free_str_array(path->pathdirs, MAX_LINE_LEN);
}

void alloc_str_array(char *strs[], int str_len, int num_strs)
{
    for (int idx = 0; idx < num_strs; idx++) {
        strs[idx] = calloc(str_len + 1, sizeof(char));
        if (strs[idx] == NULL) {
            error_and_exit();
        }
    }
}

void read_and_discard_line()
{
    char c = fgetc(stdin);
    while (c != EOF && c != '\n') {
        c = fgetc(stdin);
    }
    if (c == EOF) {
        if (feof(stdin)) {
            exit(0);
        } else {
            error_and_exit();
        }
    }
}

int read_input_line(char *buf, int max_line_len)
{
    int rc = RET_OK;

    char *ret = fgets(buf, max_line_len + 1, stdin);
    if (ret == NULL) {
        if (feof(stdin)) {
            exit(0);
        } else {
            error_and_exit();
        }
    }

    size_t input_len = strnlen(buf, max_line_len);
    if (input_len == max_line_len && buf[max_line_len - 1] != '\n') {
        read_and_discard_line();
        rc = RET_ERR;
    } else {
        buf[input_len - 1] = '\0';  // ovewrite trailing newline
    }
    return rc;
}


int tokenise(char *input, int max_line_len, char *tokens[])
{
    int numtokens = 0;
    char *token;

    char inputcpy[max_line_len];
    strcpy(inputcpy, input);

    char *next = inputcpy;
    while ((token = strsep(&next, ARG_DELIM)) != NULL) {
        if (*token != '\0') {
            strcpy(tokens[numtokens], token);
            numtokens++;
        }
    }
    return numtokens;
}

void changedirectory(char *tokens[], int numtokens)
{
    char *targetdir = NULL;
    if (numtokens == 1) {
        targetdir = getenv("HOME");
        if (targetdir == NULL) {
            print_error();
            return;
        }
    } else {
        targetdir = tokens[1];
    }

    int rc = chdir(targetdir);
    if (rc < 0) {
        printf("Error: %s\n", targetdir);
        print_error();
    }
}

void setpath(char *tokens[], int numtokens, path_t *path)
{
    for (int idx = 1; idx < numtokens; idx++) {
        strcpy(path->pathdirs[idx - 1], tokens[idx]);
    }
    path->size = numtokens - 1;
}

// Returns -1 if encounters a redirection error, 0 otherwise
int getredirect(char *tokens[], int numtokens,
                int *numcmdtokens, int *redirect_to_last_token)
{
    // The following commands should be rejected (return -1):
    //   $ ls >
    //   $ ls > out1 out2
    //   $ ls > out1 out2 out3
    //   $ ls > out1 > out2

    for (int tidx = 0; tidx < numtokens; tidx++) {
        if (strncmp(tokens[tidx], REDIRECT, sizeof(REDIRECT)) == 0 &&
            tidx != numtokens - 2) {
            return -1;
        }
    }

    if (numtokens > 2 &&
        strncmp(tokens[numtokens - 2], REDIRECT, sizeof(REDIRECT)) == 0) {
        *numcmdtokens = numtokens - 2;
        *redirect_to_last_token = 1;
    } else {
        *numcmdtokens = numtokens;
        *redirect_to_last_token = 0;
    }

    return 0;
}

// dircapacity is inclusive of terminating NUL character.
void buildpath(char *dir, char *file, int dircapacity)
{
    int dirlen = strlen(dir);
    if (dirlen > 0 && *(dir + dirlen - 1) != '/') {
        if (strlcat(dir, "/", dircapacity) >= dircapacity) {
            error_and_exit();
        }
    }

    if (strlcat(dir, file, dircapacity) >= dircapacity) {
        error_and_exit();
    }
}

int getcmdpath(char *cmd, path_t *path, char *dst, size_t dstsize)
{
    int foundpath = 0;
    char currpath[dstsize];
    for (int pidx = 0; pidx < path->size; pidx++) {
        if (strlcpy(currpath, path->pathdirs[pidx], dstsize) >= dstsize) {
            error_and_exit();
        }
        buildpath(currpath, cmd, dstsize);

        struct stat st;
        int statrc = stat(currpath, &st);
        if (statrc == 0) {
            foundpath = 1;
            break;
        }
    }

    if (foundpath) {
        if (strlcpy(dst, currpath, dstsize) >= dstsize) {
            error_and_exit();
        } 
    }
    return foundpath ? 0 : -1;
}

void print_argv(char *argv[])
{
    int idx = 0;
    while (argv[idx] != NULL) {
        printf("argv[%d]: \"%s\"\n", idx, argv[idx]);
        idx++;
    }
}

void runprog(char *tokens[], int numtokens, path_t *path)
{
    int numcmdtokens, redirect_to_last_token;
    int redirect_rc = getredirect(tokens, numtokens,
                                  &numcmdtokens, &redirect_to_last_token);
    if (redirect_rc == -1) {
        print_error();
        return;
    }

    char *cmd = tokens[0];
    int cmdpath_sz = MAXPATHLEN + 1;
    char cmdpath[cmdpath_sz];
    int getcmdpath_rc = getcmdpath(cmd, path, cmdpath, cmdpath_sz);
    if (getcmdpath_rc == -1) {
        print_error();
        return;
    }

    int argv_sz = numcmdtokens + 1;
    char *argv[argv_sz];
    int argv_str_cap = MAX_LINE_LEN + 1;
    alloc_str_array(argv, argv_str_cap, argv_sz);

    strlcpy(argv[0], cmdpath, argv_str_cap);
    for (int tidx = 1; tidx < numcmdtokens; tidx++) {
        strlcpy(argv[tidx], tokens[tidx], argv_str_cap);
    }
    // Bookending NULL ptr
    argv[numcmdtokens] = NULL;

    print_argv(argv);

    int forkrc = fork();
    if (forkrc == 0) {
        execv(cmdpath, argv);
        print_error();
    } else if (forkrc > 0) {
        int stat_loc;
        waitpid(forkrc, &stat_loc, 0);
    } else {
        print_error();
    }

    free_str_array(argv, argv_sz);
}

void runcmd(char *tokens[], int numtokens, path_t *path)
{
    char *cmd = tokens[0];
    if (strncmp(cmd, "exit", MAX_LINE_LEN) == 0) {
        exit(0);
    } else if (strncmp(cmd, "pwd", MAX_LINE_LEN) == 0) {
        char wd[MAXPATHLEN];
        getwd(wd);
        // In case of error, error message is in wd
        printf("%s\n", wd);
    } else if (strncmp(cmd, "cd", MAX_LINE_LEN) == 0) {
        changedirectory(tokens, numtokens);
    } else if (strncmp(cmd, "path", MAX_LINE_LEN) == 0) {
        setpath(tokens, numtokens, path);
    } else {
        runprog(tokens, numtokens, path);
    }
}

void free_str_array(char *strs[], int numstrs)
{
    for (int idx = 0; idx < numstrs; idx++) {
        free(strs[idx]);
    }
}

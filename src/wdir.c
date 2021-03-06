#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <pwd.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>

#define WDIR_VERSION_MAJOR 1
#define WDIR_VERSION_MINOR 0
#define WDIR_VERSION_PATCH 0

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static void
print_usage(FILE *stream)
{
    fprintf(stream, "Usage: wdir [options]\n");
    fprintf(stream, "Options:\n");
    fprintf(stream, "    -h, --help:                print this message and exit\n");
    fprintf(stream, "    -v, --verbose:             print what wdir is doing\n");
    fprintf(stream, "    -d, --dry-run:             print what wdir would do\n");
    fprintf(stream, "    -u, --no-preserve-home:    wipe user's home\n");
    fprintf(stream, "    -r, --no-preserve-root:    wipe /\n");
    fprintf(stream, "    -f, --force:               don't prompt before wipe\n");
    fprintf(stream, "    -V, --version:             print the current version and exit\n");
}

int
main(int argc, char **argv)
{
#define INPUT_BUF   2
#define PATH_SZ     2

    int arg;
    struct passwd *pw;
    char input[INPUT_BUF];
    char cwd[PATH_MAX];
    char *path[PATH_SZ];
    FTS *fts;
    FTSENT *f;

    int idx             = 0;
    bool preserve_home  = true;
    bool preserve_root  = true;
    bool force          = true;
    bool verbose        = false;
    bool dryrun         = false;

    static const struct option opts[] = {
        { "no-preserve-home",   no_argument, 0, 'u' },
        { "no-preserve-root",   no_argument, 0, 'r' },
        { "force",              no_argument, 0, 'f' },
        { "verbose",            no_argument, 0, 'v' },
        { "dry-run",            no_argument, 0, 'd' },
        { "help",               no_argument, 0, 'h' },
        { "version",            no_argument, 0, 'V' },
        { 0, 0, 0, 0 }
    };

    for (;;) {
        arg = getopt_long(argc, argv, "hvdurfV", opts, &idx);

        if (arg == -1) break;

        switch (arg) {
            case 'v':
                verbose = true;
                break;
            case 'd':
                dryrun = true;
                break;
            case 'u':
                preserve_home = false;
                break;
            case 'r':
                preserve_root = false;
                break;
            case 'f':
                force = true;
                break;
            case 'h':
                print_usage(stdout);
                exit(EXIT_SUCCESS);
            case 'V':
                printf("wdir %d.%d.%d\n", WDIR_VERSION_MAJOR, WDIR_VERSION_MINOR, WDIR_VERSION_PATCH);
                exit(EXIT_SUCCESS);
            case '?':
                print_usage(stderr);
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    if (getcwd(cwd, PATH_MAX)  == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    if (!force) {
        printf("Wipe '%s'? [y/N] ", cwd);

        fgets(input, INPUT_BUF, stdin);

        if (strcmp(input, "\n") == 0 || strcasecmp(input, "n") == 0) {
            exit(EXIT_SUCCESS);
        }
    }

    if (preserve_root && (!strcmp(cwd, "/"))) {
        fprintf(stderr, "Can't wipe '/' without --no-preserve-root.\n");
        exit(EXIT_FAILURE);
    }

    if (preserve_home) {
        pw = getpwuid(getuid());
        if (pw == NULL) {
            perror("getpwuid");
            exit(EXIT_FAILURE);
        }
        else if (!strcmp(cwd, pw->pw_dir)) {
            fprintf(stderr, "Can't wipe '%s' without --no-preserve-home.\n", cwd);
            exit(EXIT_FAILURE);
        }

    }

    path[0] = cwd;
    path[1] = NULL;

    if ((fts = fts_open(path, FTS_PHYSICAL, NULL)) == NULL) {
        perror("fts_open");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        f = fts_read(fts);

        if (errno) {
            perror("fts_read");
            continue;
        }
        else if (f == NULL) {
            break;
        }

        switch (f->fts_info) {
            case FTS_NS:
            case FTS_DNR:
            case FTS_ERR:
                fprintf(stderr, "fts_read: %s\n", strerror(f->fts_errno));
                continue;
            case FTS_F:
            case FTS_DP:

                // Ignore the current file if:
                // 1. It's the current working directory; or
                // 2. It's a hidden file.
                if (!strcmp(cwd, f->fts_path)) {
                    continue;
                } else if (f->fts_name[0] == '.') {
                    continue;
                }

                if (dryrun) {
                    printf("Would remove %s/%s\n", cwd, f->fts_name);
                    continue;
                }

                if (verbose) {
                    printf("Removing %s/%s\n", cwd, f->fts_name);
                }
                if (unlink(f->fts_name)) {
                    perror("unlink");
                }
                continue;
            default:
                continue;
        }
    }

    fts_close(fts);

    exit(EXIT_SUCCESS);
}


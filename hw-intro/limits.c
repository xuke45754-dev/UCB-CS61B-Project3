
#include <stdio.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static void print_limit(const char *label, int resource) {
    struct rlimit lim;
    if (getrlimit(resource, &lim) == -1) {
        fprintf(stderr, "%s: getrlimit failed: %s\n", label, strerror(errno));
        return;
    }

    if (lim.rlim_cur == RLIM_INFINITY)
        printf("%s: soft = unlimited", label);
    else
        printf("%s: soft = %llu", label, (unsigned long long)lim.rlim_cur);

    if (lim.rlim_max == RLIM_INFINITY)
        printf(", hard = unlimited\n");
    else
        printf(", hard = %llu\n", (unsigned long long)lim.rlim_max);
}

int main(void) {
    print_limit("stack size", RLIMIT_STACK);
    print_limit("process limit", RLIMIT_NPROC);
    print_limit("max file descriptors", RLIMIT_NOFILE);
    return 0;
}

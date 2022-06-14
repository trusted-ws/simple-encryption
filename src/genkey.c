#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <linux/sysctl.h>
#include <linux/random.h>
#include <fcntl.h>

#define MAX_PATH    255

struct arguments {
    char filename[MAX_PATH];
    size_t bytes;
};


struct arguments parse_arguments(char **argv) {

    struct arguments args; 
    int n;

    strncpy(args.filename, argv[1], MAX_PATH);
    n = atoi(argv[2]);

    if (n < 1) {
        fprintf(stderr, "Error: Bytes must be positive.\n");
        exit(1);
    }

    args.bytes = n;
    return args;
}

int get_urandom(void *buf, size_t len) {
    
    struct stat st;
    size_t i;
    int fd, cnt, flags;
    int save_errno = errno;
start:
    flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
    fd = open("/dev/urandom", flags, 0);
    if (fd == -1) {
        if (errno == EINTR)
            goto start;
        goto nodevrandom;
    }
#ifndef O_CLOEXEC
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif
    
	if (fstat(fd, &st) == -1 || !S_ISCHR(st.st_mode)) {
        close(fd);
        goto nodevrandom;
    }
    
    if (ioctl(fd, RNDGETENTCNT, &cnt) == -1) {
        close(fd);
        goto nodevrandom;
    }

    for (i = 0; i < len; i++) {
        size_t wanted = len - i;    
        ssize_t ret = read(fd, (char*)buf + i, wanted);

        if (ret == -1) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            close(fd);
            goto nodevrandom;
        }
        i += ret;
    }
    close(fd);
    errno = save_errno;
    return 0;

nodevrandom:
    errno = EIO;
    return -1;
}

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "Usage: <output> <bytes>\n");
        return 1;    
    }
    
    struct arguments args = parse_arguments(argv);

    if (args.bytes < 1) {
        fprintf(stderr, "Negative size.\n");
        return 3;
    }

    char *buffer = malloc(args.bytes);
    FILE *fp = fopen(args.filename, "w");

    if (fp == NULL) {
        perror(args.filename);
        return 2;
    }
    
    if (buffer == NULL) {
        perror("Error");
        return 1;
    }

    get_urandom(buffer, args.bytes);

    for (size_t i = 0; i < args.bytes; i++) {
        fwrite(&buffer[i], sizeof(char), 1, fp);
    }

    free(buffer);
    return 0;
}


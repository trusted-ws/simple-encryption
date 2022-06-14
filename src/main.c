#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define PATH_MAX_SIZE   255

void help_and_exit(void) {
    printf("Usage: ./simple <filename> <keyfile> <output>\n");
    exit(1);
}

struct arguments {
    char filename[PATH_MAX_SIZE];
    char keyfile[PATH_MAX_SIZE];
    char output[PATH_MAX_SIZE];
};

struct arguments parse_arguments(char **argv) {
    
    struct arguments args;

    strncpy(args.filename, argv[1], PATH_MAX_SIZE);
    strncpy(args.keyfile, argv[2], PATH_MAX_SIZE);
    strncpy(args.output, argv[3], PATH_MAX_SIZE);

    return args;
}


int file_exists(const char* filename) {
    if (access(filename, F_OK) == 0)
        return 1;
    return 0;
}

FILE *open_file_or_exit(const char* filename, const char* mode) {
    
    FILE *fp;
    
    if (mode == NULL)
        fp = fopen(filename, "rb");
    else
        fp = fopen(filename, mode);

    if (fp == NULL) {
        if (errno == ENOENT) 
            perror(filename);

        exit(ENOENT);
    }

    return fp;    
}

void encrypt(struct arguments args) {

    if (file_exists(args.output)) {
        fprintf(stderr, "Error: output file (%s) already exists!\n", args.output);
        exit(3);
    }

    int file_len, key_len, i;
    unsigned char chr, kchr, byte;

    FILE *fp = open_file_or_exit(args.filename, NULL);
    FILE *kp = open_file_or_exit(args.keyfile, NULL);
    FILE *out = open_file_or_exit(args.output, "wb");

    fseek(fp, 0, SEEK_END); fseek(kp, 0, SEEK_END);
    file_len = ftell(fp);
    key_len = ftell(kp);
    rewind(fp); rewind(kp);

    if (file_len == 0) {
        fprintf(stderr, "Error: '%s' is empty!\n", args.filename);
        exit(1);
    }

    if (key_len == 0) {
        fprintf(stderr, "Error: key file '%s' is empty!\n", args.keyfile);
        exit(1);
    }

    for (i = 0; i < file_len && (chr = getc(fp)) != EOF; i++) {

        if (ftell(kp) == key_len)
            fseek(kp, 0, SEEK_SET);

        kchr = getc(kp);
        byte = chr ^ kchr;

        fwrite(&byte, sizeof(byte), 1, out);
    }

    fclose(fp);
    fclose(kp);
    fclose(out);
}


int main(int argc, char *argv[]) {

    if (argc != 4) 
        help_and_exit();

    struct arguments args = parse_arguments(argv);
    
    encrypt(args);

    return 0;
}
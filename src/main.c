#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define VERSION         "1.0.0"
#define PATH_MAX_SIZE    255

void help_and_exit(void) {
    printf("Usage: otp <filename> <keyfile> <output> [-f]\n");
    exit(1);
}

void fullhelp_and_exit(void) {
    printf(" OTP version %s\n\n"
           " Required arguments:\n"
           "   <filename>       Path of file to be encrypted.\n"
           "   <keyfile>        Path of key file.\n"
           "   <output>         Output filename.\n\n"
           " Unique arguments:\n"
           "   --help           Show this message.\n"
           "   --version        Show the version.\n\n"
           " Optional arguments:\n"
           "   -f               Force overwriting when the output file exists.\n\n"
            , VERSION);
    exit(0);

}

struct arguments {
    char filename[PATH_MAX_SIZE];
    char keyfile[PATH_MAX_SIZE];
    char output[PATH_MAX_SIZE];
    _Bool force_overwriting;
};

struct arguments parse_arguments(size_t argc, char **argv) {
    
    struct arguments args;
    args.force_overwriting = 0;

    for (int i = 0; i < argc; i++) {

        if (strcmp(argv[i], "-f") == 0)
            args.force_overwriting = 1;

        if (strcmp(argv[i], "--version") == 0) {
            printf("OTP version %s\n", VERSION);
            exit(0);
        }

        if (strcmp(argv[i], "--help") == 0)
            fullhelp_and_exit();
    }

    if (argc < 4)
        help_and_exit();

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

    if (!args.force_overwriting) {
        if (file_exists(args.output)) {
            fprintf(stderr, "Error: output file (%s) already exists!\n", args.output);
            exit(3);
        }
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

    struct arguments args = parse_arguments(argc, argv);
    
    encrypt(args);

    return 0;
}

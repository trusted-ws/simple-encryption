#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "otpfile.h"

#define VERSION         "2.0.0"
#define PATH_MAX_SIZE    255

void help_and_exit(void) {
    printf("Simple OTP %s (c) 2022-2025 trusted-ws \n"
           "Usage: otp <filename> <keyfile> <output> [options]\n", VERSION);
    exit(1);
}

void fullhelp_and_exit(void) {
    printf("\n Simple OTP %s (c) 2022-2025 trusted-ws \n\n"
           " Usage: otp <filename> <keyfile> <output> [options]\n\n"
           " Required arguments:\n"
           "   <filename>             Path of file to be encrypted.\n"
           "   <keyfile>              Path of key file.\n"
           "   <output>               Output filename.\n\n"
           " Unique arguments:\n"
           "   --show <filename>      Show details of encrypted file (OTP only).\n"
           "   --help / -h            Show this message.\n"
           "   --version              Show the version.\n\n"
           " Optional arguments:\n"
           "   --force / -f           Force overwriting when the output file exists.\n"
           "   --description <text>   Set a description text to the encrypted file.\n"
           "   --no-warnings          Suppress warning messages during execution.\n"
           "   --ignore-otp-filetype  Ignore OTP filetype header during encryption.\n\n"
            " Notes:\n"
            "   If the key size is smaller than the plaintext, the key will be reused cyclically.\n\n"
            "   You are free to use this software as you wish, but we highly recommend following\n"
            "   best practices whilst using OTP, such as employing a truly random key as long as\n"
            "   plaintext, never reusing keys, securely distributing and storing keys, and\n"
            "   destroying them after use.\n\n"
           , VERSION);
    exit(0);
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

struct arguments {
    char filename[PATH_MAX_SIZE];
    char keyfile[PATH_MAX_SIZE];
    char output[PATH_MAX_SIZE];
    char description[HEADER_DESCRIPTION_MAX_SIZE];
    _Bool force_overwriting;
    _Bool ignore_otp_filetype;
    _Bool no_warnings;
};

struct arguments parse_arguments(size_t argc, char **argv) {
    
    struct arguments args;
    args.force_overwriting = 0;
    args.ignore_otp_filetype = 0;
    args.no_warnings = 0;

    strcpy(args.description, "");

    for (int i = 0; i < argc; i++) {

        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0)
            args.force_overwriting = 1;

        if (strcmp(argv[i], "--version") == 0) {
            printf("OTP version %s\n", VERSION);
            exit(0);
        }

        if (strcmp(argv[i], "--show") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "\"--show\" requires a filename!\n");
                fullhelp_and_exit();
            }
            FILE *fp = open_file_or_exit(argv[i+1], NULL);
            struct otp_file_header header = read_header(fp);
            print_header(header);
            fclose(fp);
            exit(0);
        }

        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
            fullhelp_and_exit();

        if (strcmp(argv[i], "--description") == 0) {
            if (i + 1 >= argc) { help_and_exit(); }
            strncpy(args.description, argv[i+1], HEADER_DESCRIPTION_MAX_SIZE);
        }

        if (strcmp(argv[i], "--ignore-otp-filetype") == 0) {
            args.ignore_otp_filetype = 1;
        }

        if (strcmp(argv[i], "--no-warnings") == 0) {
            args.no_warnings = 1;
        }
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

void encrypt(struct arguments args) {

    if (!args.force_overwriting) {
        if (file_exists(args.output)) {
            fprintf(stderr, "Error: output file (%s) already exists!\n", args.output);
            exit(3);
        }
    }

    long unsigned file_len, key_len, i;
    unsigned char chr, kchr, byte;
    unsigned int header_offset = 0;

    FILE *fp = open_file_or_exit(args.filename, NULL);
    FILE *kp = open_file_or_exit(args.keyfile, NULL);
    FILE *out = open_file_or_exit(args.output, "wb");

    /* If OTP filetype header is preset we will assume it is 
       a decrypt operation unless ignore-otp-filetype was set.

       In this case we must calculate the header offset and always
       consdier it when performing the operation.
    */
   _Bool has_header = is_header_present(fp);  // auto reset file cursor!

    if (!args.ignore_otp_filetype) {
        if (!has_header) {
            struct otp_file_header header = create_file_header(args.filename, args.description);
            write_header(out, &header);
        } else {
            header_offset = sizeof(struct otp_file_header);
        }
    }    

    fseek(fp, 0, SEEK_END); fseek(kp, 0, SEEK_END);
    file_len = ftell(fp) - header_offset;
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

    if (!args.no_warnings) {
        if (file_len > key_len) {
            fprintf(stderr,
            "Warning: key length (%zu bytes) does not match file length (%zu bytes). "
            "This may compromise OTP security.\n", key_len, file_len);
        }
    }

    fseek(fp, header_offset, SEEK_CUR);

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

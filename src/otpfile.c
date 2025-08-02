#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "otpfile.h"


struct otp_file_header create_file_header(const char *filename,
                                          const char *description)
{
    struct otp_file_header header;
    const char MAGIC_NO[4] = SIMPLE_OTP_MAGIC;
    
    memset(&header, 0, sizeof(header));
    memcpy(&header.magic, MAGIC_NO, sizeof(header.magic));
    header.version = SIMPLE_OTP_VERSION;

    strncpy(header.original_filename, filename, sizeof(header.original_filename));
    strncpy(header.description, description, sizeof(header.description));

    header.timestamp = time(NULL);

    return header;
}

_Bool is_header_present(FILE *stream)
{
    const char MAGIC_NO[4] = SIMPLE_OTP_MAGIC;
    char buffer[4];

    long cursor = ftell(stream);
    if (cursor == -1L) { perror("ftell"); }

    fseek(stream, 0, SEEK_SET);
    size_t bytes_read = fread(buffer, sizeof(char), sizeof(MAGIC_NO), stream);
    fseek(stream, cursor, SEEK_SET);

    if (bytes_read != sizeof(buffer)) {
        return 0;
    }

    if (memcmp(MAGIC_NO, buffer, sizeof(MAGIC_NO)) == 0) {
        return 1;
    }
    return 0;
}

int write_header(FILE *stream, struct otp_file_header *header)
{
    if (!stream) { return 1; }
    fwrite(header, sizeof(*header), 1, stream);
    
    return 0;
}

struct otp_file_header read_header(FILE* stream)
{
    struct otp_file_header header;
    long cursor = ftell(stream);

    fseek(stream, 0, SEEK_SET);
    fread(&header, sizeof(header), 1, stream);
    fseek(stream, cursor, SEEK_SET);

    return header;
}

void print_header(struct otp_file_header header)
{
    const char _magic[4] = SIMPLE_OTP_MAGIC;

    if (memcmp(header.magic, _magic, sizeof(_magic)) != 0) {
        fprintf(stderr, "Error: Invalid file format. Expected OTP magic, but got: "
                        "0x%02X 0x%02X 0x%02X 0x%02X\n",
                (unsigned char)header.magic[0],
                (unsigned char)header.magic[1],
                (unsigned char)header.magic[2],
                (unsigned char)header.magic[3]);
        return;
    }
    
    char time_buffer[80];
    struct tm *time_info = localtime(&header.timestamp);

    if (time_info != NULL) {
        strftime(time_buffer, sizeof(time_buffer), "%a %Y-%m-%d %H:%M:%S %Z", time_info);
    } else {
        snprintf(time_buffer, sizeof(time_buffer), "Invalid timestamp");
    }

    printf("Header Information:\n");
    printf("  Magic:       \"%c%c%c%c\" (0x%02X 0x%02X 0x%02X 0x%02X)\n",
        header.magic[0], header.magic[1], header.magic[2], header.magic[3],
        (unsigned char)header.magic[0], (unsigned char)header.magic[1],
        (unsigned char)header.magic[2], (unsigned char)header.magic[3]);
    printf("  Version:     0x%02X\n", header.version);
    printf("  Filename:    %s\n", header.original_filename);
    printf("  Description: %s\n", header.description);
    printf("  Timestamp:   %lu (%s)\n", (unsigned long)header.timestamp, time_buffer);
    printf("  Header size: %zu bytes\n", sizeof(header));
}


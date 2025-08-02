#ifndef _OTP_FILETYPE_
#define _OTP_FILETYPE_

#define SIMPLE_OTP_MAGIC            {0x7f, 0x4f, 0x54, 0x50}
#define HEADER_FILENAME_MAX_SIZE    64
#define HEADER_DESCRIPTION_MAX_SIZE 128
#define SIMPLE_OTP_VERSION          0x01

#pragma pack(push, 1)
struct otp_file_header
{
    char magic[4];
    char version;
    char __reserved1[3];
    char original_filename[HEADER_FILENAME_MAX_SIZE];
    char description[HEADER_DESCRIPTION_MAX_SIZE];
    unsigned long timestamp;
};
#pragma pack(pop)

struct otp_file_header create_file_header(const char* filename, const char* description);
struct otp_file_header read_header(FILE* stream);
int write_header(FILE *stream, struct otp_file_header *header);
void print_header(struct otp_file_header header);
_Bool is_header_present(FILE *stream);


#endif /* _OTP_FILETYPE_ */
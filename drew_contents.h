#ifndef CREATE 
#define CREATE 

#include "util.h"

int create(int argc, char *argv[], int VFLAG, int SFLAG);
char * create_header(char * pathname, int file_type, int VFLAG);
void add_symlink_or_file(char * pathname, FILE * archive_file, 
    struct stat sb, int VFLAG);
void fill_header_name(HeaderPtr header, 
    char *pathname, int VFLAG);
void split_pathname(HeaderPtr header, char *pathname);
void fill_header_uid( HeaderPtr header, struct stat sb);
void fill_header_size( HeaderPtr header, int file_type, struct stat sb);
void fill_header_typeflag( HeaderPtr header, int file_type);
int fill_header_linkname(char * pathname,  HeaderPtr header, int VFLAG);
void fill_header_checksum(char * buffer, HeaderPtr header);
void write_data_blocks(char * pathname, FILE *archive_file, char * header);
void set_null_blocks(char buffer[BLOCK_SIZE]);
int insert_special_int(char *where, size_t size, int32_t val);
void write_end_blocks(FILE * archive_file);
void v_flag(struct stat sb, int VFLAG, char* path);
int get_inodenum(char * path);
void add_dir(char * pathname, FILE * archive_file, 
    struct stat sb, int VFLAG);
void traverse_dir(char * pathname, FILE* archive_file, int VFLAG);

#endif

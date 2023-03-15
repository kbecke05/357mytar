#include "create.h"

/*create main*/

int create(int argc, char *argv[], int VFLAG, int SFLAG) {
    int i;
    struct stat sb;
    FILE * archive_file;
    char * write_perimission = "w+";
    /*open a new file to make the archive in*/
    archive_file = fopen_file_error_check(argv[2], write_perimission);

    /*for each path name that is given, deal with it*/
    for (i= 3; i < argc; i++) {
        /*use lstat to get the file type, and only continue on success*/
        if (lstat(argv[i], &sb)!= -1) {
            /*if verbose, print out the current path name*/
            v_flag(sb, VFLAG, argv[i]);
            /*deal with each file type*/
            if (S_ISLNK(sb.st_mode) || S_ISREG(sb.st_mode)) {
                add_symlink_or_file(argv[i], archive_file, sb, VFLAG);
            }
            else if (S_ISDIR(sb.st_mode)) {
                add_dir(argv[i], archive_file, sb, VFLAG);
            }
        }
    }

    /*write two null blocks to signify end of archive*/
    write_end_blocks(archive_file);
    fclose(archive_file);
    return 0;
}

/*driver methods*/

void add_symlink_or_file(char * pathname, FILE * archive_file, 
    struct stat sb, int VFLAG) {
        char * headerbuf;
        int file_type;
        /*create and write the header*/
        if S_ISLNK(sb.st_mode) {
            file_type = SYMLINK;
        }
        else {
            file_type = REGFILE;
        }
        headerbuf = create_header(pathname, file_type, VFLAG);
        fwrite(headerbuf, BLOCK_SIZE, 1, archive_file);
        /*if it's a regular file, add the data blocks too*/

        if(file_type == REGFILE) {
            write_data_blocks(pathname, archive_file, headerbuf);
        }

        /*if v option is set, insert a new line*/

        if(VFLAG == 1) {
            printf("\n");
        }

        free(headerbuf);
}

void traverse_dir(char * pathname, FILE* archive_file, int VFLAG) {
    DIR * dir;
    struct dirent * pdirent;
    int currinode, pinode;
    char* newpath;
    struct stat sb;
    char * slash = "/";

    if((dir = opendir(pathname))== NULL) {
        printf("Error opening directory %s\n", pathname);
        return;
    }

    /*read each entry from the directory*/
    while((pdirent = readdir(dir))) {
        currinode = get_inodenum(".");
        pinode = get_inodenum("..");

        /*make sure we aren't at the root*/
        if(currinode != pinode) {

            /*skip current and parent directories*/
            if (strcmp(pdirent -> d_name, ".") == 0 || 
                strcmp(pdirent -> d_name, "..") == 0) {
                    continue;
            }
            /*malloc enough memory for the new path + slash + null*/
            newpath = malloc_error_check(strlen(pathname) 
                + strlen(pdirent->d_name) +2);
            /*copy current pathname, add slash, add curr dir name*/
            strcpy(newpath, pathname);
            strcat(newpath, slash);
            strcat(newpath, pdirent -> d_name);

            /*lstat the new path to get the filetype*/
            if(lstat(newpath, &sb)==-1) {
                perror("lstat");
                exit(EXIT_FAILURE);
            }

            /*call VFLAG method if needed*/
            v_flag(sb, VFLAG, newpath);

            /*now deal with each file type*/
            if (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode)) {
                add_symlink_or_file(newpath, archive_file, sb, VFLAG);
            }
            else if (S_ISDIR(sb.st_mode)) {
                add_dir(newpath, archive_file, sb, VFLAG);
            }

            free(newpath);
        }  
    }
    closedir(dir);
}

void add_dir(char * pathname, FILE * archive_file, 
    struct stat sb, int VFLAG) {
        char * fullpath;
        char * header;
        char * slash = "/";
        /*duplicate the current path name*/
        fullpath = strdup(pathname);

        /*realloc enough memory for the slash and a null char*/
        fullpath = realloc(fullpath, strlen(pathname)+2);

        /*add in the slash*/
        strcat(fullpath,slash);

        /*generate a header for the directory based on the new full path*/
        header = create_header(fullpath, DIRECTORY, VFLAG);

        /*write header into archive file*/
        fwrite(header, BLOCK_SIZE, 1, archive_file);

        if(VFLAG) {
            printf("\n");
        }

        free(header);
        free(fullpath);

        /*now traverse the directory to get everything beneath it*/

        traverse_dir(pathname, archive_file, VFLAG);

}

char * create_header(char * pathname, int file_type, int VFLAG) {
    struct stat sb;
    HeaderPtr header;
    char * buffer;

    /*allocate mem for arr of 512 chars -> (sizeof(char)=1)*/
    buffer = (char *) calloc_error_check(BLOCK_SIZE, 1);

    /*allocate mem for header*/
    header = calloc_error_check(1, sizeof(Header));

    if(lstat(pathname, &sb)==-1) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    /*fill in name part of the header*/
    fill_header_name(header, pathname, VFLAG);
    /*fill in mode part of the header*/
    sprintf(header->mode, "%07o", sb.st_mode & 07777);
    /*fill in uid*/
    fill_header_uid(header, sb);
    /*fill in gid*/
    sprintf(header->gid, "%07o", sb.st_gid);
    /*fill in size*/
    fill_header_size(header, file_type, sb);
    /*fill in mtime*/
    sprintf(header->mtime, "%011lo", sb.st_mtime);
    /*fill in typeflag*/
    fill_header_typeflag(header, file_type);
    /*if symlink, fill in linkname*/
    if (file_type == SYMLINK) {
        if (fill_header_linkname(pathname, header, VFLAG) == 1) {
            return buffer;
        }
    }
    /*now fill in everything else in the header struct*/
    strncpy(header->magic, MAGIC_VAL, MAGIC_SIZE);
    strncpy(header->version, VERSION_VAL, VERSION_SIZE);

    strcpy(header->uname, getpwuid(sb.st_uid)->pw_name);
    strcpy(header->gname, getgrgid(sb.st_gid)->gr_name);

    /*now write everything in the header struct to the buffer*/
    memcpy(buffer + NAME, header->name, NAME_SIZE);
    memcpy(buffer + MODE, header->mode, MODE_SIZE);
    memcpy(buffer + UID, header->uid, UID_SIZE);
    memcpy(buffer + GID, header->gid, GID_SIZE);
    memcpy(buffer + SIZE, header->size, SIZE_SIZE);
    memcpy(buffer + MTIME, header->mtime, MTIME_SIZE);
    memcpy(buffer + CHKSUM, header->chksum, CHKSUM_SIZE);
    memcpy(buffer + TYPEFLAG, header->typeflag, TYPEFLAG_SIZE);
    memcpy(buffer + LINKNAME, header->linkname, LINKNAME_SIZE);
    memcpy(buffer + MAGIC, header->magic, MAGIC_SIZE);
    memcpy(buffer + VERSION, header->version, VERSION_SIZE);
    memcpy(buffer + UNAME, header->uname, UNAME_SIZE);
    memcpy(buffer + GNAME, header->gname, GNAME_SIZE);
    memcpy(buffer + DEVMAJOR, header->devmajor, DEVMAJOR_SIZE);
    memcpy(buffer + DEVMINOR, header->devminor, DEVMINOR_SIZE);
    memcpy(buffer + PREFIX, header->prefix, PREFIX_SIZE);

    /*fill in checksum*/
    fill_header_checksum(buffer, header);
    memcpy(buffer + CHKSUM, header->chksum, CHKSUM_SIZE);

    free(header);
    return buffer;
}

/*functions for filling in and writing the header*/

void fill_header_name(HeaderPtr header, 
    char *pathname, int VFLAG) 
{
    if (strlen(pathname)<NAME_SIZE) {
        strncpy(header->name, pathname, NAME_SIZE);
    }
    else {
        /*if the name is too long, we need to split it
        between the prefix and the name attributes*/
        split_pathname(header, pathname);
    }

    if(VFLAG) {
        printf("Can't split pathname");
    }
}

void split_pathname( HeaderPtr header, char *pathname) {
    int i,j;
    
    i = strlen(pathname) - (NAME_SIZE + 1);
    for(j = i; j<strlen(pathname); j++) {
        if(pathname[j] == '/') {
            if (NAME_SIZE >= strlen(&pathname[j+1])) {
                strncpy(header->name, &pathname[j+1], NAME_SIZE);
                break;
            }
            return;
        }
    }
    if (j<=PREFIX_SIZE){
        if (pathname[j+1]== '/') {
            memcpy(header->prefix, pathname, j+1);
        }
        else {
            memcpy(header->prefix, pathname, j);
        }
        return;
    }
}

void fill_header_uid( HeaderPtr header, struct stat sb) {
    /*need to check the size of the uid*/
    if (sb.st_uid > 07777777) {
        insert_special_int(header->uid, 8, sb.st_uid);
    }
    else {
        sprintf(header->uid, "%07o", sb.st_uid);
    }
}

void fill_header_size( HeaderPtr header, int file_type, struct stat sb) {
    /*if its a regular file, store octal string size, else size is empty*/
    if (file_type == REGFILE) {
        sprintf(header->size, "%011lo", (long)sb.st_size);
    }
    else {
        strcpy(header->size, "00000000000");
    }
}

void fill_header_typeflag( HeaderPtr header, int file_type) {
    if (file_type == DIRECTORY) {
        header->typeflag[0]= '5';
    }
    else if (file_type == SYMLINK) {
        header->typeflag[0]= '2';
    }
    else if (file_type == REGFILE) {
        header->typeflag[0]= '0';
    }
    else {
        printf("Unknown file type");
    }
}

int fill_header_linkname(char * pathname,  HeaderPtr header, int VFLAG) {
    if(strlen(pathname)> LINKNAME_SIZE) {
        if (VFLAG) {
            printf("Linkname too long");
        }
        return 1;
    }
    readlink(pathname, header->linkname, LINKNAME_SIZE);
    return 0;
}

void fill_header_checksum(char * buffer,  HeaderPtr header) {
    /*calculate checksum*/
    unsigned int checksum = 0;
    unsigned int i;
    
    for(i=0; i<BLOCK_SIZE; i++) {
        checksum += (unsigned char)buffer[i];
    }
    checksum += CHKSUM_SIZE *SPACE;
    /*put result into the header file*/
    sprintf(header->chksum, "%07o", checksum);
}

/*functions to write the data blocks*/

void write_data_blocks(char * pathname, FILE *archive_file, char * header) {
    FILE * path_file;
    char buffer[BLOCK_SIZE];
    char * readpermission = "r";

    path_file = fopen_file_error_check(pathname, readpermission);
    set_null_blocks(buffer);

    /*read from file into buffer of BLOCK_SIZE*/
    while(fread(buffer, BLOCK_SIZE, 1, path_file)>0) {
        if (!feof(path_file)) {
            /*write the block to the file*/
            fwrite(buffer, BLOCK_SIZE, 1, archive_file);
            /*reset the entire block to 0*/
            memset(buffer, 0, BLOCK_SIZE);
        }
    }
    /*take care of the last block*/
    fwrite(buffer, BLOCK_SIZE, 1, archive_file);
    fclose(path_file);
}

void set_null_blocks(char buffer[BLOCK_SIZE]) {
    int i;
    for (i=0; i<BLOCK_SIZE; i++) {
        buffer[i] = '\0';
    }
}

/*helper methods*/

int insert_special_int(char *where, size_t size, int32_t val) {
    int err = 0;
    if (val < 0 || (size < sizeof(val)))
    {
        err++;
    }
    else
    {
        memset(where, 0, size);
        *(int32_t *)(where + size - sizeof(val)) = htonl(val);
        *where |= 0x80;
    }
    return err;
}

void write_end_blocks(FILE * archive_file) {
    int i;
    char buffer[BLOCK_SIZE];

    /*set everything in the block to nulls*/

    set_null_blocks(buffer);

    /*write a block of nulls twice*/
    for (i=0; i< 2; i++) {
        fwrite(buffer, BLOCK_SIZE, 1, archive_file);
    }
}

void v_flag(struct stat sb, int VFLAG, char* path) {
    if (VFLAG) {
        if (S_ISDIR(sb.st_mode)) {
            printf("%s/", path);
        }
        else {
            printf("%s", path);
        }
    }
}

int get_inodenum(char * path) {
    DIR * dir;
    struct dirent *ent;
    struct stat sb;
    if ((dir = opendir(path))) {
        if((ent = readdir(dir)) == NULL) {
            perror("can't read dir");
        }
        else {
            if ((lstat(path, &sb)) == -1) {
                perror(path);
            }
            else {
                rewinddir(dir);
                closedir(dir);
                return sb.st_ino;
            }
        }
    }
    return -1;
}
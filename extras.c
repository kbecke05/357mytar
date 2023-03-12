void traverse_dir(char * pathname, FILE* archive_file, int VFLAG) {
    DIR * dir;
    struct dirent * pdirent;
    int currinode, pinode;
    char* newpath;

    dir = opendir_error_check(pathname);

    /*read each entry from the directory*/
    while(dirent = readdir(dir)) {
        currinode = get_inodenum(".");
        pinode = get_inodenum("..");

        /*skip current and parent directories*/
        if (strcmp(dirent -> d_name, ".") == 0 || 
            strcmp(dirent -> d_name, "..") == 0) {
                continue;
        /*make sure we aren't at the root*/
        if(currinode != pinode) {
            /*malloc enough memory for the new path + slash + null*/
            newpath = malloc_error_check(strlen(pathname) 
                + strlen(dirent->d_name) +2);
            /*copy current pathname, add slash, add curr dir name*/
            strcpy(newpath, pathname);
            strcat(newpath, '/');
            strcat(newpath, dirrent -> d_name);
            v_flag()
        }
            

        
    }
}
}

void add_dir(char * pathname, FILE * archive_file, 
    struct stat sb, int VFLAG) {
        char * fullpath;
        char * header;
        /*duplicate the current path name*/
        fullpath = strdup(pathname);

        /*realloc enough memory for the slash and a null char*/
        fullpath = realloc(fullpath, strlen(pathname)+2);

        /*add in the slash*/
        strcat(pathname, '/');

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
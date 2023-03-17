#ifndef CONTENTS
#define CONTENTS

#include "util.h"

void iterate_arch(int fd, int vflag, char *args[], int num_args,
                int specific, int extract, int sflag);
void write_permissions(char mode[], int fd, unsigned offset);
char get_type(int fd, unsigned offset);
void write_owner_group(int fd, unsigned offset);
long int get_size(int fd, unsigned offset);
void write_mtime(int fd, unsigned offset);
char *write_uid(int fd, unsigned offset);
char *write_gid(int fd, unsigned offset);
char *get_prefix(int fd, unsigned offset);
char *get_mtime(int fd, unsigned offset);
char *get_xname(char *name);



#endif

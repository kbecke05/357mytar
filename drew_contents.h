#ifndef CONTENTS
#define CONTENTS

#include "util.h"

void list_files(int fd, int vflag, char *given, int specific, int extract);
void write_permissions(char mode[], int fd, unsigned offset);
char get_type(int fd, unsigned offset);
void write_owner_group(int fd, unsigned offset);
long int get_size(int fd, unsigned offset);
void write_mtime(int fd, unsigned offset);
char *write_uid(int fd, unsigned offset);
char *write_gid(int fd, unsigned offset);



#endif

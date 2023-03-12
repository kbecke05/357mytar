#include "util.h"
#include "create.h"

#ifndef CONTENTS
#include "contents.h"
#endif

#ifndef EXTRACT
#include "extract.h"
#endif



int main(int argc, char * argv[]) {
  int CFLAG=0, TFLAG=0, XFLAG=0, VFLAG=0, SFLAG=0;
  int i;

  /*check for not enough arguments*/
  if (argc < 3) {
    fprintf(stderr, "Not enough arguments\n");
    fprintf(stderr, 
        "usage: %s mytar [ctxvS]f tarfile [ path [ ... ] ] \n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /*set all flags*/
  for(i=0; i< strlen(argv[1]); i++) {
    switch (argv[1][i])
    {
      case 'c':
        CFLAG = 1;
        break;
      case 't':
        TFLAG = 1;
        break;
      case 'x':
        XFLAG = 1;
        break;
      case 'v':
        VFLAG = 1;
        break;
      case 'S':
        SFLAG = 1;
        break;
      case 'f':
        continue;
      default:
        fprintf(stderr, "Wrong options\n");
        fprintf(stderr,
          "usage: %s mytar [ctxvS]f tarfile [ path [ ... ] ] \n", argv[0]);
        exit(EXIT_FAILURE);
        break;
    }
  }

  /*check for multiple flags*/

  if ((CFLAG + TFLAG + XFLAG) > 1) {
    fprintf(stderr, "Only one of c, t, x options allowed\n");
    fprintf(stderr,
       "usage: %s mytar [ctxvS]f tarfile [ path [ ... ] ] \n", argv[0]);
        exit(EXIT_FAILURE);
  }

  /*check for no options*/
  if ((CFLAG + TFLAG + XFLAG) ==0) {
      fprintf(stderr, "One of c, t, x options required\n");
      fprintf(stderr, 
        "usage: %s mytar [ctxvS]f tarfile [ path [ ... ] ] \n", argv[0]);
          exit(EXIT_FAILURE);
    }
  /*call functions*/

  if (CFLAG) {
    create(argc, argv, VFLAG, SFLAG);
  }
  /*if (TFLAG) {
    contents(argc, argv, VFLAG, SFLAG);
  }
  if (XFLAG) {
    extract(argc, argv, VFLAG, SFLAG);
  }*/

  return 0;

}

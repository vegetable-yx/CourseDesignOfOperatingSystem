#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/sysinfo.h"

int
main(int argc, char *argv[])
{
  if(argc != 1){
    fprintf(2, "sysinfo.c wants 0 parameter, but %d parameters provided\n", argc - 1);
    exit(1);
  }
  
  struct sysinfo si;
  if (sysinfo(&si) < 0) {
    exit(1);
  }
  printf("free memory nums: %d, process nums: %d\n", si.freemem, si.nproc);
  exit(0);
}
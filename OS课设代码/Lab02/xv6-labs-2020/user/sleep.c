#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    fprintf(2, "sleep.c wants 1 parameter, but %d parameters provided\n", argc - 1);
    exit(1);
  }
  int time = atoi(argv[1]);
  if (time <= 0) exit(1);
  sleep(time);
  exit(0);
}
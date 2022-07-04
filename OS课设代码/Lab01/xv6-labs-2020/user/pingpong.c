#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 1) {
    fprintf(2, "pingpong.c wants 0 parameter, but received %d parameters\n", argc - 1);
    exit(1);
  }

  int p1[2];
  int p2[2];
  
  pipe(p1);
  pipe(p2);

  if (fork() == 0) {
    close(p1[1]);
    close(p2[0]);

    char buffer[16];
    if (read(p1[0], buffer, 1) == 1) {
      printf("%d: received ping\n", getpid());
      write(p2[1], buffer, 1);
      exit(0);
    }
  } else {
    close(p1[0]);
    close(p2[1]);

    char buffer[16];
    write(p1[1], "p", 1);
    read(p2[0], buffer, 1);
    printf("%d: received pong\n", getpid());
  }

  exit(0);
}
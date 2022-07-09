#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void child(int p[]) {
    close(p[1]);
    int mark;
    if (read(p[0], &mark, sizeof(mark)) == 0) return;
    printf("prime %d\n", mark);

    int t;
    int p1[2];
    pipe(p1);
    if (fork() == 0) {
        child(p1);
    } else {
        close(p1[0]);
        while(read(p[0], &t, sizeof(t)) > 0) {
            if (t % mark != 0) {
                write(p1[1], &t, sizeof(t));
            }
        }
        close(p1[1]);
        close(p[0]);
        wait(0);
    }

    exit(0);
}

int
main(int argc, char *argv[])
{
  if(argc != 1){
    fprintf(2, "primes.c wants 0 parameter, but %d parameters provided\n", argc - 1);
    exit(1);
  }
  
  int p[2];
  pipe(p);

  if (fork() == 0) {
    child(p);
  } else {
    close(p[0]);
    int i;
    for (i = 2; i <= 35; i++) {
        write(p[1], &i, sizeof(i));
    }
    close(p[1]);
    wait(0);
  }

  exit(0);
}
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
  char xargs[MAXARG], buf[MAXARG];
  char *cmd[MAXARG];
  char *p = buf;
  int count = 0, cur = 0;

  for (int i = 1; i < argc; i++) cmd[count++] = argv[i];
  int k;
  while ((k = read(0, xargs, sizeof xargs)) > 0) {
    for (int i = 0; i < k; i++) {
        if (xargs[i] == '\n') {
            buf[cur] = 0;
            cmd[count++] = p;
            cmd[count] = 0;
            count = argc - 1;
            cur = 0;
            if (fork() == 0) exec(argv[1], cmd);
            wait(0);
        } else if (xargs[i] == ' ') {
            buf[cur++] = 0;
            cmd[count++] = p;
            p = &buf[cur];
        } else {
            buf[cur++] = xargs[i];
        }
    }
  }

  exit(0);
}

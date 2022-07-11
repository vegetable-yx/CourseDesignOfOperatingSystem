#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// return the filename without parent directory.
char*
fmtname(char *path)
{
  char *p;
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
    
  return p;
}

// find all files in the path.
void find(char* path, char* name) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    return;
  }

  if(fstat(fd, &st) < 0){
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    if (strcmp(fmtname(path), name) == 0) {
        printf("%s\n", path);
    }
    break;

  case T_DIR:
    strcpy(buf, path);

    // add '/' to the end of the path.
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) // not recurse into . and ..
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        continue;
      }

      find(buf, name);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "find.c wants 2 parameters, but %d parameters provided.\n", argc - 1);
    exit(1);
  }
  
  find(argv[1], argv[2]);
  exit(0);
}
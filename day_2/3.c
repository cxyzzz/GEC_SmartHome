#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
  int fd;
  fd = open("t",O_RDONLY);
  
  if(fd == -1)
  {
    perror("open error!\n");
  }
  
  char buf[200] = {0};
  int r = read(fd, buf, 10);
  printf("r = %d\n", r);
  printf("%s\n", buf);
  close(fd);
  
  return 0;
}

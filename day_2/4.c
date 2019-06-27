#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
  int i = 0, fd1, fd2, r1, r2;
  
  fd1 = open("1.txt",O_RDONLY);
  fd2 = open("../2.txt",O_WRONLY);

  if(fd1 == -1 || fd2 == -1)
  {
    perror("open 1.txt or 2.txt error!");
    return 0;
  }
  
  char buf[5] = {0};
  do
  {
    r1 = read(fd1, buf, 4);
    buf[r1] = '\0';
    printf("%s", buf); 
    r2 = write(fd2, buf, r1);
  }
  while (r1 != 0);

  close(fd1);
  close(fd2);

  return 0;
}

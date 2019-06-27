#include <stdio.h>
#include <math.h>

int main()
{
  int a, tmp, i = 1, b[5] = {0};
  printf("input a\n");
  scanf("%d", &a);
  tmp = a;
  
  while(tmp != 0)
  {
    b[i] = tmp%10;
    tmp = tmp/10;
    i++;
  }
  
  
  tmp = pow(b[1],i) + pow(b[2],i) + pow(b[3],i);
  if(tmp == a)
  {
    printf("是水仙花数\n");
  }
  else
  {
    printf("不是水仙花数\n");
  }
  
  return 0;
}
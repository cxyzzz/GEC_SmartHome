#include <stdio.h>

int main()
{
  int a;
  printf("input a\n");
  scanf("%d", &a);
  
  if (a%2 == 0)
  {
    printf("不是奇数\n");
  }
  else
  {
    printf("是奇数\n");
  }
  
  return 0;
}
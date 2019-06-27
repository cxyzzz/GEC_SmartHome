# include <stdio.h>

int main()
{
  int a;
  int t = 1;
  printf("input a\n");
  scanf("%d", &a);
  
  if(a < 2)
  {
      printf("不是素数\n");
  }
  else
  {
      for(int i =2; i < a; i++)
      {
        if(a%i == 0)
        {
          printf("不是素数\n");
          return 0;
        }
      }
      printf("是素数\n");
  }
  return 0;
}
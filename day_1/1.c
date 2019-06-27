# include <stdio.h>

int main()
{
  int a, b;
  printf("input a,b\n");
  scanf("%d%d", &a, &b);
  
  printf("最大值：%d\n", a>b?a:b);
  
  return 0;
}
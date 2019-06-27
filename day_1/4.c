#include <stdio.h>

void swap(int *a, int *b)
{
  int tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

int main()
{
  int a,b;
  printf("input a,b\n");
  scanf("%d%d", &a, &b);
  printf("交换前：a = %d，b= %d\n", a,b);

  swap(&a, &b);  
  printf("交换后：a = %d，b= %d\n", a,b);

  return 0;
}
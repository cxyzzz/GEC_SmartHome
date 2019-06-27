#include <stdio.h>

int main()
{
    int i, a = 12345, m[10];
    for(i = 0; a!=0; i++)
    {
        m[i] = a%10;
        a /= 10;
        printf("%d\n", m[i]);
    }
    
    printf("------%d", i);
}

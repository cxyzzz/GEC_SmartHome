#include <stdio.h>
#include <iostream>
#include <string.h>
using namespace std;
int main()
{
    const char * s = "./";
    const char * f = ".bmp";

    for(int i = 1; i <= 7; i++)
    {
        char * buf = new char[strlen(s) + 1];
        sprintf(buf, "%s%d", s, i);
        char *buf2 = new char[strlen(buf) + 1];
        sprintf(buf2, "%s%s", buf, f);
        printf("%s\n", buf2);
    }
}
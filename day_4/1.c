#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "data.h"

int * p, blue = 0x000000FF, black = 0x001F2F3F, bg = 0x00FFFFFF;

void display_point(int x, int y, int color)
{
    *(p + x * 800 + y) = color;
}

void display_bg()
{
    int i, j;
    for(i = 0; i < 480; i++)
    {
        for(j = 0; j < 800; j++)
        {
            display_point(i, j, bg);
        }
    }    
}

void display_word(int color,int x0, int y0, char word[], int w, int h)
{
    int i, j, x, y;
    
    for(i = 0; i < w*h/8; i++)
    {
        for(j = 0; j < 8; j++)
        {
            if(word[i]>>(7-j) & 1 ==1)
            {
                x = i/(w/8) + x0;
                y = (i%(w/8))*8 + j + y0;
                display_point(x, y, color);
            }
        }
    }
}

void display_digit(int color, int x0, int y0, int number, int w, int h)
{
    int i, m = 0, a[100];
    for(i = 0; number != 0; i++)
    {
        a[i] = number%10;
        number /= 10;
    }
    for(int j = i - 1; j >= 0; j--)
    {
        // printf("%d\n", a[j]);    
        display_word(blue, x0, y0+m*8, digits[a[j]], w, h);
        m++;
    }   
}

int main()
{
    int lcd_fd;
    lcd_fd = open("/dev/fb0", O_RDWR);
    if(lcd_fd == -1)
    {
        perror("open /dev/fb0 error!\n");
    }
    
    p = mmap(NULL, 480*800*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    
    display_bg();
    display_word(black, 0, 0, chen, 16, 16);
    display_word(black, 0, 16, xin, 16, 16);
    display_word(black, 0, 32, yu, 16, 16);
    display_digit(blue, 0, 56, 18, 8, 16);
    
    int number;
    printf("please input display number:\n");
    scanf("%d", &number);
    display_digit(blue, 16, 0, number, 8, 16);
    
    munmap(p, 480*800*4);
    close(lcd_fd);
    
    return 0;
}
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
using namespace std;

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

void display_picture(int x0, int y0, char file[])
{
    printf("%s\n", file);
    short int m;
    int bmp_fd, w, h;
    bmp_fd = open(file, O_RDONLY);
    
    if(bmp_fd == -1)
    {
        perror("open picture file error!");
        return;
    }
    
    lseek(bmp_fd, 0x12, SEEK_SET);
    read(bmp_fd, &w, 4);
    lseek(bmp_fd, 0x16, SEEK_SET);
    read(bmp_fd, &h, 4);    
    lseek(bmp_fd, 0x1C, SEEK_SET);
    read(bmp_fd, &m, 2);
    
    printf("w = %d\n", w);
    printf("h = %d\n", h);
    printf("m = %d\n", m);
    
    char color_buf[w*h*m/8];
    lseek(bmp_fd, 54, SEEK_SET);
    read(bmp_fd, color_buf, w*h*m/8);
    
    int n = 0;
    for(int i = h-1+x0; i >= x0; i--)
    {
        for(int j = y0; j < w + y0; j++)
        {
           int color = color_buf[2+3*n] << 16 | color_buf[1+3*n] << 8 | color_buf[0+3*n];
           display_point(i, j, color);
           n++;
        }
    }
    
    close(bmp_fd);
}

int main()
{
    int lcd_fd;
    lcd_fd = open("/dev/fb0", O_RDWR);
    if(lcd_fd == -1)
    {
        perror("open /dev/fb0 error!\n");
        return 0;
    }
     
    p = (int *)mmap(NULL, 480*800*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
 
    // display_bg();
   // char file[100];
   // printf("please input picture file: \n");
   // scanf("%s", file);
   // int x, y;
   // printf("input x0, y0\n");
   // scanf("%d%d", &x, &y);
    
    while(1)
    {
        const char * s = "./";
        const char * f = ".bmp";
        for(int i = 1; i <= 7; i++)
        {
            char * buf = new char[strlen(s) + 1];
            sprintf(buf, "%s%d", s, i);
            char *buf2 = new char[strlen(buf) + 1];
            sprintf(buf2, "%s%s", buf, f);
        
            display_bg();
            sleep(3);
            display_picture(0, 0, buf2);
        }
    }
    
    munmap(p, 480*800*4);
    close(lcd_fd);
    
    return 0;
}
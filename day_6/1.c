#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/input.h>
#include <strings.h>
#include <stdlib.h>

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

void display_circle(int x, int y)
{
    int i, j;
    for(i = 0; i < 480; i++)
    {
        for(j = 0; j < 800; j++)
        {
            if((i - x)*(i - x)+(j - y)*(j - y) <= 10*10)
            {
                display_point(i, j, blue);
            }
            else
            {
                display_point(i, j, bg);
            }
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

struct ts
{
    int x;
    int y;
};

/*
 * Event types
 
#define EV_SYN			0x00
#define EV_KEY			0x01
#define EV_REL			0x02
#define EV_ABS			0x03


* Absolute axes

#define ABS_X			0x00
#define ABS_Y			0x01
#define ABS_Z			0x02
*/

void ts_read(struct ts *ts_sp)
{
    int fd = open("/dev/input/event0", O_RDONLY);
    if(fd == -1)
    {
        perror("open error!\n");
        return;
    }
    
    struct input_event buf;
    bzero(&buf, sizeof(buf));
    
    int count = 0;
    
    while(1)
    {
        read(fd, &buf, sizeof(buf));
        if(buf.type == EV_ABS)
        {
            if(buf.code == ABS_X)
            {
                ts_sp->x = buf.value;
                count++;
            }
            if(buf.code == ABS_Y)
            {
                ts_sp->y = buf.value;
                count++;
            }
            if(count == 2)
            {
                break;
            }
        }
    }
    
    close(fd);
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
     
    p = mmap(NULL, 480*800*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    
    display_bg();
 
    struct ts *ts_sp = malloc(sizeof(struct ts));
    while(1)
    {
        ts_read(ts_sp);
        printf("(%d, %d)\n", ts_sp->x, ts_sp->y);
        //display_circle(ts_sp->y, ts_sp->x);
        // display_bg();
        if(ts_sp->x + 20 < 800 && ts_sp->x - 20 >= 0 && ts_sp->y + 40 < 480 && ts_sp->y - 20 >= 0)
        {
            display_picture(ts_sp->y - 20, ts_sp->x - 20, "./s.bmp");
        }
    }
    
    free(ts_sp);
    munmap(p, 480*800*4);
    close(lcd_fd);
    return 0;
}
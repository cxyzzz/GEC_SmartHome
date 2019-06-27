#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/input.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int * p, blue = 0x000000FF, black = 0x001F2F3F, bg = 0x00FFFFFF;
char * picture[] = {"/hut/1.bmp","/hut/2.bmp", "/hut/3.bmp", "/hut/4.bmp", "/hut/5.bmp", "/hut/6.bmp", "/hut/7.bmp"}; 

// 画点    
void display_point(int x, int y, int color)
{
    *(p + x * 800 + y) = color;
}

// 显示背景色
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

// 显示圆
void display_circle(int x, int y)
{
    int i, j;
    for(i = 0; i < 480; i++)
    {
        for(j = 0; j < 800; j++)
        {
            if((i - x)*(i - x)+(j - y)*(j - y) <= 5*5)
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

// 显示图片
void display_picture(int x0, int y0, char file[])
{
    display_bg();
    
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
/*
// 手动拼接路径显示图片
void loop_display_picture(int i)
{       
    const char * s = "/hut/";
    const char * f = ".bmp";
    
    char * buf = malloc(strlen(s) + 1);
    sprintf(buf, "%s%d", s, i);
    char *buf2 = malloc(strlen(buf) + 1);
    sprintf(buf2, "%s%s", buf, f);
        
    display_bg();        
    display_picture(0, 0, buf2);
            
    free(buf);
    free(buf2);
}
*/

// 坐标结构体
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
    
    int i = 0;      // 图片文件名
    struct ts start, end; 
    // loop_display_picture(i);
    display_picture(0, 0, picture[i]);
    
    while(1)
    {
        int r = read(fd, &buf, sizeof(buf));
        
        if(r != sizeof(buf))
        {
            continue;
        }
        
        // 读取触摸屏幕事件坐标
        if(buf.type == EV_ABS)
        {
            if(buf.code == ABS_X)
            {
                ts_sp->x = buf.value;
                if(start.x == -1)
                {
                    start.x = ts_sp->x;
                }
                end.x = ts_sp->x;
            }
            else if(buf.code == ABS_Y)
            {
                ts_sp->y = buf.value; 
                if(start.y == -1)
                {
                    start.y = ts_sp->y;
                }
                end.y = ts_sp->y;
            }
        }
        
        // 手指离开屏幕后进行数据处理
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            if(abs(end.x - start.x) > abs(end.y - start.y))
            {
                if(end.x > start.x)
                {
                    printf("right\n");
                    if(i == 0)
                    {
                        i = 6;
                    }
                    else
                    {
                        i--;
                    }
                    // loop_display_picture(i);
                    display_picture(0, 0, picture[i]);
                }
                else if(end.x < start.x)
                {
                    printf("left\n");
                    if(i + 1 > 6)
                    {
                        i = 0;
                    }
                    else
                    {
                        i++;
                    }
                    // loop_display_picture(i);
                    display_picture(0, 0, picture[i]);
                    
                }
            }
            else if(abs(end.x - start.x) < abs(end.y - start.y))
            {
                if(end.y > start.y)
                {
                    printf("down\n");
                    if(i == 0)
                    {
                        i = 6;
                    }
                    else
                    {
                        i--;
                    }
                    // loop_display_picture(i);
                    display_picture(0, 0, picture[i]);
                }
                else if(end.y < start.y)
                {
                    printf("up\n");
                    if(i + 1 > 6)
                    {
                        i = 0;
                    }
                    else
                    {
                        i++;
                    }
                    // loop_display_picture(i);
                    display_picture(0, 0, picture[i]);
                }
            }
        
        printf("start:(%d, %d),end:(%d, %d)\n\n", start.x, start.y, end.x, end.y);
        
        start.x = -1;
        start.y = -1; 
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
 
    struct ts *ts_sp = malloc(sizeof(struct ts));
    ts_read(ts_sp);
    
    free(ts_sp);
    munmap(p, 480*800*4);
    close(lcd_fd);
    return 0;
}
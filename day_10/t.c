#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <math.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "data.h"       // 存储点阵数据
#include <pthread.h>

//6818串口所对应的文件名
#define COM2 "/dev/ttySAC1"
#define COM3 "/dev/ttySAC2"
#define COM4 "/dev/ttySAC3"

int * p, blue = 0x000000FF, black = 0x001F2F3F, white = 0x00FFFFFF;
char * picture[] = {"/hut/1.bmp","/hut/2.bmp", "/hut/3.bmp", "/hut/4.bmp", "/hut/5.bmp", "/hut/6.bmp"}; 

void display_point(int x, int y, int color)
{
    *(p + x * 800 + y) = color;
}

void display_bg(int x0, int y0,int x1, int y1 , int color)
{
    int i, j;
    for(i = x0; i < x1; i++)
    {
        for(j = y0; j < y1; j++)
        {
            display_point(i, j, color);
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
    int i, j, m = 0, a[100] = {0};
    if(number == 0)
    {
        display_word(blue, x0, y0, digits[0], w, h);
    }
    else
    {
        for(i = 0; number != 0; i++)
        {
            a[i] = number%10;
            number /= 10;
        }
        for(j = i - 1; j >= 0; j--)
        {
           // printf("%d\n", a[j]);    
            display_word(blue, x0, y0+m*8, digits[a[j]], w, h);
            m++;
        } 
    }
}


void display_picture(int x0, int y0, char file[])
{
    printf("display picture %s\n", file);
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
    
   // printf("w = %d\n", w);
   // printf("h = %d\n", h);
   // printf("m = %d\n", m);
    
    char color_buf[w*h*m/8];
    lseek(bmp_fd, 54, SEEK_SET);
    read(bmp_fd, color_buf, w*h*m/8);
    
    int i, j, n = 0;
    for(i = h-1+x0; i >= x0; i--)
    {
        for(j = y0; j < w + y0; j++)
        {
           int color = color_buf[2+3*n] << 16 | color_buf[1+3*n] << 8 | color_buf[0+3*n];
           display_point(i, j, color);
           n++;
        }
    }
    
    close(bmp_fd);
}

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

void * ts_read(void *arg)
{
    int x0 = 120, y0 = 400;
    struct ts *ts_sp = malloc(sizeof(struct ts));

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
    display_bg(x0, y0, 480, 800, white);
    display_picture(x0, y0, picture[i]);
    
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
                        i = 5;
                    }
                    else
                    {
                        i--;
                    }
                    display_bg(x0, y0, 480, 800, white);
                    display_picture(x0, y0, picture[i]);
                }
                else if(end.x < start.x)
                {
                    printf("left\n");
                    if(i + 1 > 5)
                    {
                        i = 0;
                    }
                    else
                    {
                        i++;
                    }
                    display_bg(x0, y0, 480, 800, white);
                    display_picture(x0, y0, picture[i]);
                    
                }
            }
            else if(abs(end.x - start.x) < abs(end.y - start.y))
            {
                if(end.y > start.y)
                {
                    printf("down\n");
                    if(i == 0)
                    {
                        i = 5;
                    }
                    else
                    {
                        i--;
                    }
                    display_bg(x0, y0, 480, 800, white);
                    display_picture(x0, y0, picture[i]);
                }
                else if(end.y < start.y)
                {
                    printf("up\n");
                    if(i + 1 > 5)
                    {
                        i = 0;
                    }
                    else
                    {
                        i++;
                    }
                    display_bg(x0, y0, 480, 800, white);
                    display_picture(x0, y0, picture[i]);
                }
            }
        
        printf("start:(%d, %d),end:(%d, %d)\n\n", start.x, start.y, end.x, end.y);
        
        start.x = -1;
        start.y = -1; 
        }
    }
    
    close(fd);}

int serial_init(char *file,int speed)
{
	/*
		打开串口文件
	*/
    int fd = open(file,O_RDWR);
    if(fd == -1)
    {
        perror("open serial error");
        return -1;
    }
	
    /*定义串口属性结构体*/
    struct termios myserial;
    memset(&myserial,0,sizeof(myserial));//清零
	
	/*设置控制模式 本地连接  使能接收*/
    myserial.c_cflag |= (CLOCAL | CREAD); 
    myserial.c_cflag &=  ~CSIZE; //清空数据位
    myserial.c_cflag &= ~CRTSCTS; //无硬件控制流
    myserial.c_cflag |= CS8; //数据位 8
    myserial.c_cflag &= ~CSTOPB; //停止位 1
    myserial.c_cflag &= ~PARENB; //不要校验

    /*设置波特率*/
	switch(speed)
	{
		case 9600:
			cfsetospeed(&myserial,B9600);
			cfsetispeed(&myserial,B9600);
			break;
		case 57600:
			cfsetospeed(&myserial,B57600);
			cfsetispeed(&myserial,B57600);
			break;
		case 115200:
			cfsetospeed(&myserial,B115200);
			cfsetispeed(&myserial,B115200);
			break;
	}
	
    /*刷新输出队列，清除正接收的数据*/
    tcflush(fd,TCIFLUSH);

    /*更改配置*/
    tcsetattr(fd,TCSANOW,&myserial);
	
    return fd;
}

void * yanwu(void *arg)
{
    int fd = serial_init(COM2, 9600);
    int fd1 = open("/dev/beep", O_RDWR);

    if(fd == -1)
    {
        perror("serial_init error!\n");
        return (void *)-1;
    }
    if(fd1 == -1)
    {
        perror("open /dev/beep error!\n");
        return (void *)-1;
    }
    
    while(1)
    {
        char w_buf[9] = {0xFF, 0X01, 0x86, 0, 0, 0, 0, 0, 0x79};
        write(fd, w_buf, 9);
        usleep(1000);
        char r_buf[9];
        int r = read(fd, r_buf, 9);
        if(r == 9 && r_buf[0] == 0xff && r_buf[1] == 0x86)
        {
            int n = r_buf[2]<<8 | r_buf[3];
            printf("n = %d\n", n);
            if(n > 138)
            {
                ioctl(fd1, 0, 1);
                sleep(3);
                ioctl(fd1, 1, 1);
            }
            else
            {
                ioctl(fd1, 1, 1);
            }
        }
        
        sleep(2);
    }
    
    close(fd1);
    close(fd);
    return 0;
}
void * gy_39(void *arg)
{
/*    
	发送的信息：
		开发板发送给传感器的指令格式：帧头+指令+校验和(8bit)
		帧头和校验和是用来“验证身份”的
		指令才是用来表达此次通信的目的
		各占一个字节，共三个字节
		帧头：0xa5
		指令：
				只请求光照： 10000001 -> 0x81
				只请求温湿度气压海拔：10000010 -> 0x82
				两种都请求：10000011 -> 0x83
		检验和：
				等于帧头加上指令，只保存低8位
			0xa5 + 0x81  -> 0x26
*/
   int fd = serial_init(COM3, 9600);
   int m = 2;
    while(1)
    {

        char w_buf[][3] ={{0xa5, 0x81, 0x26}, {0xa5, 0x82, 0x27}, {0xa5, 0x83, 0x28}};
        int Lux = 0, T = 0, P = 0, Hum = 0, H = 0;
        int t = write(fd, w_buf[m], 3);
        // printf("%d\n", t);
        usleep(1000);
    
        char r_buf[24];
        int r = read(fd, r_buf, 24);
        // printf("%d\n", r);
        if(r == 9 && r_buf[2] == 0x15)
        {
            Lux = (r_buf[4]<<24 | r_buf[5]<<16 | r_buf[6]<<8 | r_buf[7])/100;
        }
        else if(r == 15 && r_buf[2] == 0x45)
        {
            T = (r_buf[4]<<8 | r_buf[5])/100;
            P = (r_buf[6]<<24 | r_buf[7]<<16 | r_buf[8]<<8 | r_buf[9])/100;
            Hum = (r_buf[10]<<8 | r_buf[11])/100;
            H = (r_buf[12]<<8 | r_buf[13])/100;
        }
        else if(r == 24)
        {
            Lux = (r_buf[4]<<24 | r_buf[5]<<16 | r_buf[6]<<8 | r_buf[7])/100;
            T = (r_buf[13]<<8 | r_buf[14])/100;
            P = (r_buf[15]<<24 | r_buf[16]<<16 | r_buf[8]<<8 | r_buf[9])/100;
            Hum = (r_buf[10]<<8 | r_buf[11])/100;
            H = (r_buf[12]<<8 | r_buf[13])/100;
        }
        printf("Lux = %d, T = %d, P = %d, Hum = %d, H = %d\n", Lux, T, P, Hum, H);
        sleep(1);
        
            if(Lux < 6)
            {
                display_picture(130, 200, "/hut/on.bmp");
            }
            else
            {
                display_picture(130, 200, "/hut/off.bmp");
            }
            
            display_bg(270, 0, 480, 200, white);
            
            // 光强
            display_word(black, 270, 10, guan, 16, 16);
            display_word(black, 270, 26, qian, 16, 16);
            display_word(black, 270, 42, maohao, 16, 16);            
            display_digit(blue, 270, 58, Lux, 8, 16);
            display_word(blue, 270, 82, L, 8, 16);
            display_word(blue, 270, 90, U, 8, 16);
            display_word(blue, 270, 98, X, 8, 16);
            
            // 温度
            display_word(black, 296, 10, wen, 16, 16);
            display_word(black, 296, 26, du, 16, 16);
            display_word(black, 296, 42, maohao, 16, 16);
            display_digit(blue, 296, 58, T, 8, 16);
            display_word(blue, 296, 82, C, 16, 16);
 
            // 气压
            display_word(black, 348, 10, qi, 16, 16);
            display_word(black, 348, 26, ya, 16, 16);
            display_word(black, 348, 42, maohao, 16, 16);
            display_digit(blue, 348, 58, P, 8, 16);   
            display_word(blue, 348, 106, _P, 8, 16);
            display_word(blue, 348, 114, A, 8, 16);
 
            // 湿度
            display_word(black, 322, 10, shi, 16, 16);
            display_word(black, 322, 26, du, 16, 16);
            display_word(black, 322, 42, maohao, 16, 16);
            display_digit(blue, 322, 58, Hum, 8, 16);
            display_word(blue, 322, 90, baifenhao, 8, 16);
            
            // 海拔
            display_word(black, 372, 10, hai, 16, 16);
            display_word(black, 372, 26, ba, 16, 16);
            display_word(black, 372, 42, maohao, 16, 16);
            display_digit(blue, 372, 58, H, 8, 16);  
            display_word(blue, 372, 82, M, 8, 16);
    }
    
    close(fd);
    return 0;
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
    display_bg(0, 0, 480, 800, white);

	display_word(0x0a0a0a,125,16,chen, 16, 16);//名字
	display_word(0x0a0a0a,125,32,ton,16, 16);
	display_word(0x0a0a0a,125,48,xin,16,16);
	
	display_word(0x0a0a0a,145,32,tong1,16,16);//班级
	display_word(0x0a0a0a,145,48,yi,16,16);
	display_word(0x0a0a0a,145,64,maohao,16,16);
    
	display_digit(0x0000ee,145,80,26,8,16);//学号
	
	display_word(0x0a0a0a,173,16,chen,16,16);//名字
	display_word(0x0a0a0a,173,32,xin,16,16);
	display_word(0x0a0a0a,173,48,yu,16,16);
	
	display_word(0x0a0a0a,193,32,tong1,16,16);//班级
	display_word(0x0a0a0a,193,48,er,16,16);
	display_word(0x0a0a0a,193,64,maohao,16,16);
	
	display_digit(0x0000ee,193,80,18,8,16);//学号
	
	display_word(0x0a0a0a,221,16,mei,16,16);//名字
	display_word(0x0a0a0a,221,32,miao,16,16);
	
	display_word(0x0a0a0a,241,32,tong1,16,16);//班级
	display_word(0x0a0a0a,241,48,yi,16,16);
	display_word(0x0a0a0a,241,64,maohao,16,16);
	
	display_digit(0x0000ee,241,80, 43,8,16);//学号
	
	display_word(0x0a0a0a,25,216,zhi,48,48);//智能家居系统
	display_word(0x0a0a0a,25,280,neng,48,48);
	display_word(0x0a0a0a,25,344,jia,48,48);
	display_word(0x0a0a0a,25,408,ju,48,48);
	display_word(0x0a0a0a,25,472,xi,48,48);
	display_word(0x0a0a0a,25,536,tong,48,48);


    pthread_t t0, t1, t2;
    
    /*if(pthread_create(&t0, NULL, yanwu, NULL) == -1)
    {
        perror("fail to create pthread t0\n");
        return -1;
    }*/
    if(pthread_create(&t1, NULL, gy_39, NULL) == -1)
    {
        perror("fail to create pthread t1\n");
        return -1;
    }
    /*
    struct ts *ts_sp = malloc(sizeof(struct ts));
    ts_read(50, 400,ts_sp);
    */
    if(pthread_create(&t2, NULL, ts_read, NULL) == -1)
    {
        perror("fail to create pthread t2\n");
        return -1;
    }
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    
    
    munmap(p, 480*800*4);
    close(lcd_fd);
    return 0;
}
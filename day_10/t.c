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

// 6818 串口所对应的文件名
#define COM2 "/dev/ttySAC1"
#define COM3 "/dev/ttySAC2"
#define COM4 "/dev/ttySAC3"

/*
p ，整型指针，屏幕帧缓冲驱动文件内存映射后的指针
toggle 整型，LED 灯手动开关控制标志
red blue gray black white 整型，RGB 颜色
picture 字符型数组指针，存储所有图片路径
*/
int * p, toggle = 0 ,red = 0x00FF0000, blue = 0x000000FF,gray = 0x00AFBFCF, black = 0x001F2F3F, white = 0x00FFFFFF;
char * picture[] = {"/hut/1.bmp","/hut/2.bmp", "/hut/3.bmp", "/hut/4.bmp", "/hut/5.bmp", "/hut/6.bmp"}; 

/*  画点函数
x 整型，纵向坐标
y 整型，横向坐标
*/
void display_point(int x, int y, int color)
{
    *(p + x * 800 + y) = color;
}

/*  绘制背景色
x0 整型，起点纵向坐标；y0 整型，起点横向坐标
x1 整型，终点纵向坐标；y1 整型，终点横向坐标
color 整型，所画背景颜色
*/
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

/*  画实心圆
x0 整型，起点纵向坐标；y0 整型，起点横向坐标
x 整型，圆心纵向坐标；y 整型，圆心横向坐标
color 整型，所画圆颜色
*/
void display_circle(int x0, int y0, int x, int y, int r, int color)
{
    int i, j;
    for(i = x0; i < 480; i++)
    {
        for(j = y0; j < 800; j++)
        {
            if((i - x)*(i - x)+(j - y)*(j - y) <= r * r)
            {
                display_point(i, j, color);
            }
            else
            {
                display_point(i, j, white);
            }
        }
    }
}

/*  显示字符
color 整型，所画字符颜色
x0 整型，起点纵向坐标；y0 整型，起点横向坐标
word 字符型数组，存储所画字符点阵数据
w 整型，字符宽度；h 整型，字符高度
w*h/8 字符点阵数据个数，一个点阵数据 8 位
*/
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

/*  显示任意数字（最大 19 位）
color 整型，所画数字颜色
x0 整型，起点纵向坐标；y0 整型，起点横向坐标
number 长整型，所画数字
w 整型，数字字符宽度；h 整型，数字字符高度
*/
void display_digit(int color, int x0, int y0, long long int number, int w, int h)
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

/*  显示位图
x0 整型，起点纵向坐标；y0 整型，起点横向坐标
file 字符型数组，存储所画图片路径
*/
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
    
    lseek(bmp_fd, 0x12, SEEK_SET);  // 将光标移动到存储位图宽度处
    read(bmp_fd, &w, 4);    // 读取位图宽度
    lseek(bmp_fd, 0x16, SEEK_SET);  // 将光标移动到存储位图高度处
    read(bmp_fd, &h, 4);    // 读取位图高度    
    lseek(bmp_fd, 0x1C, SEEK_SET);  // 将光标移动到存储位图图像色深处
    read(bmp_fd, &m, 2);    // 读取位图图像色深
    
   // printf("w = %d\n", w);
   // printf("h = %d\n", h);
   // printf("m = %d\n", m);
    
    char color_buf[w*h*m/8];    // w*h*m/8 计算位图字节数
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
        usleep(500);
    }
    
    close(bmp_fd);
}

// 坐标结构体
struct ts
{
    int x;
    int y;
};

/*  屏幕触摸函数，控制相册图片的切换和 LED 灯手动开关
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
    int x0 = 130, y0 = 400;
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
    display_bg(x0, y0, 480, 800, white);
    display_picture(x0, y0, picture[i]);
    display_circle(400, 0, 440, 50, 30, gray);        // 显示 LED 灯开关

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
            if(start.x >= 400 && start.y >= 120)    // 限制只有在图片区域的滑动才能切换图片
            {
                if(abs(end.x - start.x) > abs(end.y - start.y))     // 判断左右滑
                {
                    if(end.x > start.x)
                    {
                        printf("right\n");
                        if(i -1 < 0)
                        {
                            i = sizeof(picture)/sizeof(char *) - 1;
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
                        if(i + 1 > sizeof(picture)/sizeof(char *) - 1)
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
                else if(abs(end.x - start.x) < abs(end.y - start.y))        // 判断上下滑
                {
                    if(end.y > start.y)
                    {
                        printf("down\n");
                        if(i -1 < 0)
                        {
                            i = sizeof(picture)/sizeof(char *) - 1;
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
                        if(i + 1 > sizeof(picture)/sizeof(char *) - 1)
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
        
            }
            
        printf("start:(%d, %d),end:(%d, %d)\n\n", start.x, start.y, end.x, end.y);
        
        // LED 手动开关
        if(start.x >= 0 && end.x <= 100 && start.y >= 400 && end.y <= 480)
        {
            toggle = ~toggle;
            
            if(toggle)
            {
                printf("toggle = %d\n", toggle);
                display_picture(130, 200, "/hut/on.bmp");
                usleep(500);
                display_circle(400, 0, 440, 50, 30, red);        // LED 灯开关打开
            }
            else
            {
                printf("toggle = %d\n", toggle);
                display_picture(130, 200, "/hut/off.bmp");
                usleep(500);
                display_circle(400, 0, 440, 50, 30, gray);        // LED 灯开关关闭
            }        
        }
        
        start.x = -1;
        start.y = -1; 
        }
    }
    
    close(fd);
}

/*  串口初始化
file 字符型指针，存储串口文件路径
speed 整型，存储串口波特率
*/
int serial_init(char *file,int speed)
{
	/* 打开串口文件 */
    int fd = open(file,O_RDWR);
    if(fd == -1)
    {
        perror("open serial error");
        return -1;
    }
	
    /* 定义串口属性结构体 */
    struct termios myserial;
    memset(&myserial,0,sizeof(myserial));// 清零
	
	/* 设置控制模式 本地连接  使能接收 */
    myserial.c_cflag |= (CLOCAL | CREAD); 
    myserial.c_cflag &=  ~CSIZE; // 清空数据位
    myserial.c_cflag &= ~CRTSCTS; // 无硬件控制流
    myserial.c_cflag |= CS8; // 数据位 8
    myserial.c_cflag &= ~CSTOPB; // 停止位 1
    myserial.c_cflag &= ~PARENB; // 不要校验

    /* 设置波特率 */
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
	
    /* 刷新输出队列，清除正接收的数据 */
    tcflush(fd,TCIFLUSH);

    /* 更改配置 */
    tcsetattr(fd,TCSANOW,&myserial);
	
    return fd;
}
/* 烟雾传感器 */
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
            if(n > 138)     // 烟雾浓度超过一定值蜂鸣器报警
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

/*  GY-39 传感器
*/
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
        
        // 当关照强度低于一定值且手动开关关闭时打开灯
        if(Lux < 6 && toggle == 0)
        {
            display_picture(130, 200, "/hut/on.bmp");
        }
        else if(Lux >= 6 && toggle == 0)
        {
            display_picture(130, 200, "/hut/off.bmp");
        }
            
        display_bg(270, 0, 400, 200, white);
            
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


// 显示姓名班级学号等信息
void display_info()
{
    display_picture(5,2,"/hut/hut_logo.bmp");    // HUT logo
    display_picture(5,55,"/hut/gec_logo.bmp");    // GEC logo

	display_word(0x0a0a0a,25,216,zhi,48,48);//智能家居系统
	display_word(0x0a0a0a,25,280,neng,48,48);
	display_word(0x0a0a0a,25,344,jia,48,48);
	display_word(0x0a0a0a,25,408,ju,48,48);
	display_word(0x0a0a0a,25,472,xi,48,48);
	display_word(0x0a0a0a,25,536,tong,48,48);
	display_word(0x0a0a0a,25,536,tong,48,48);

	display_word(0x0a0a0a,125,16,chen, 16, 16);//名字
	display_word(0x0a0a0a,125,32,ton,16, 16);
	display_word(0x0a0a0a,125,48,xin,16,16);
	
	display_word(0x0a0a0a,145,32,tong1,16,16);//班级
	display_word(0x0a0a0a,145,48,yi,16,16);
	display_word(0x0a0a0a,145,64,maohao,16,16);
    
	display_digit(blue,145,80,16408200126LL,8,16);//学号
	
	display_word(0x0a0a0a,173,16,chen,16,16);//名字
	display_word(0x0a0a0a,173,32,xin,16,16);
	display_word(0x0a0a0a,173,48,yu,16,16);
	
	display_word(0x0a0a0a,193,32,tong1,16,16);//班级
	display_word(0x0a0a0a,193,48,er,16,16);
	display_word(0x0a0a0a,193,64,maohao,16,16);
	
	display_digit(blue,193,80,16408200218LL,8,16);//学号
	
	display_word(0x0a0a0a,221,16,mei,16,16);//名字
	display_word(0x0a0a0a,221,32,miao,16,16);
	
	display_word(0x0a0a0a,241,32,tong1,16,16);//班级
	display_word(0x0a0a0a,241,48,yi,16,16);
	display_word(0x0a0a0a,241,64,maohao,16,16);
	
	display_digit(blue,241,80, 16408200143LL,8,16);//学号
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
     
    p = mmap(NULL, 480*800*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);   // 映射屏幕帧缓冲文件到内存，加快读写速度
    display_bg(0, 0, 480, 800, white);


    display_info();
    
    pthread_t t0, t1, t2;
    
    if(pthread_create(&t0, NULL, yanwu, NULL) == -1)
    {
        perror("fail to create pthread t0\n");
        return -1;
    }
    if(pthread_create(&t1, NULL, gy_39, NULL) == -1)
    {
        perror("fail to create pthread t1\n");
        return -1;
    }
    if(pthread_create(&t2, NULL, ts_read, NULL) == -1)
    {
        perror("fail to create pthread t2\n");
        return -1;
    }
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    munmap(p, 480*800*4);
    close(lcd_fd);
    return 0;
}
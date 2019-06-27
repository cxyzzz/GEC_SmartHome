#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
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

// 设置波特率
int speed_arr[] = {B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400};
int name_arr[] = {0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}; 
void set_speed(int fd, int speed)
{
    int i, status;

    struct termios opt;
    tcgetattr(fd, &opt);
    
    for(i = 0; i < sizeof(speed_arr)/sizeof(int); i++)
    {
        if(speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&opt, speed_arr[i]);
            cfsetospeed(&opt, speed_arr[i]);
            
            status = tcsetattr(fd, TCSANOW, &opt);
            if(status == -1)
            {
                perror("tcsetattr error!\n");
                return;
            }
            
            tcflush(fd, TCIOFLUSH);
        }
    }
}

// 设置数据位、停止位、校验类型
int set_parity(int fd, int databits, int stopbits, int parity)
{
    struct termios opt;
    if(tcgetattr(fd, &opt) == -1)
    {
        perror("\n");
        return -1;
    }

    opt.c_cflag &= ~CSIZE;      // 清空数据位
    
    // 设置数据位
    switch(databits)
    {
        case 7:
            opt.c_cflag |= CS7;
            break;
        case 8:
            opt.c_cflag |= CS8;
            break;
        default:
            printf("Unsupported data size\n");
            return -1;
    }

    // 设置校验位
    switch(parity)
    {
        case 'n':
        case 'N':
            opt.c_cflag &= ~PARENB;
            opt.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            opt.c_cflag |= (PARODD | PARENB);
            opt.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E':
            opt.c_cflag |= PARENB;
            opt.c_cflag &= ~PARODD;
            opt.c_iflag |= INPCK;
            break;
        case 's':
        case 'S':
            opt.c_cflag &= ~PARENB;
            opt.c_cflag &= ~CSTOPB;
            break;
        default:
            printf("Unsupported parity\n");
            return -1;
    }

    // 设置停止位
    switch(stopbits)
    {
        case 1:
            opt.c_cflag &= ~CSTOPB;
            break;
        case 2:
            opt.c_cflag |= CSTOPB;
            break;
        default:
            printf("Unsupported stop bits\n");
            return -1;
    }
    
    // 设置输入校验选项
    if(parity != 'n')
    {
        opt.c_iflag |= INPCK;
    }
    tcflush(fd, TCIFLUSH);
    opt.c_cc[VTIME] = 150;      // 设置超时 150s
    opt.c_cc[VMIN] = 0;     // 立即更新选项
    if(tcsetattr(fd, TCSANOW, &opt) == -1)
    {
        perror("error!\n");
        return -1;
    }
    opt.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opt.c_oflag &= ~OPOST;
    return 0;
}

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

void * yanwu(void)
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

void * gy_39(void)
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
        int Lux = 0, T, P, Hum, H;
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
        
        if(Lux > 6)
        {
            display_picture(120, 456, "/hut/on.bmp");
        }
        else
        {
            display_picture(120, 456, "/hut/off.bmp");
        }
        display_word(black, 170, 100, guan, 16, 16);
        display_word(black, 170, 116, qian, 16, 16);
        display_word(black, 170, 132, maohao, 16, 16);
        display_bg(170, 148, 186, 164, white);
        display_digit(blue, 170, 148, Lux, 8, 16);
        display_word(blue, 170, 169, L, 8, 16);
        display_word(blue, 170, 177, U, 8, 16);
        display_word(blue, 170, 185, X, 8, 16);

        display_word(black, 196, 100, wen, 16, 16);
        display_word(black, 196, 116, du, 16, 16);
        display_word(black, 196, 132, maohao, 16, 16);
        display_bg(196, 148, 218, 172, white);
        display_digit(blue, 196, 148, T, 8, 16);
        display_word(blue, 196, 169, C, 16, 16);

        display_word(black, 222, 100, shi, 16, 16);
        display_word(black, 222, 116, du, 16, 16);
        display_word(black, 222, 132, maohao, 16, 16);
        display_bg(222, 148, 238, 182+100, white);
        display_digit(blue, 222, 148, Hum, 8, 16);
        display_word(blue, 222, 174, baifenhao, 8, 16);

        display_word(black, 248, 100, qi, 16, 16);
        display_word(black, 248, 116, ya, 16, 16);
        display_word(black, 248, 132, maohao, 16, 16);
        display_bg(248, 148, 264, 209+100, white);
        display_digit(blue, 248, 148, P, 8, 16);   
        display_word(blue, 248, 193, _P, 8, 16);
        display_word(blue, 248, 201, A, 8, 16);

        display_word(black, 272, 100, hai, 16, 16);
        display_word(black, 272, 116, ba, 16, 16);
        display_word(black, 272, 132, maohao, 16, 16);
        display_bg(272, 148, 288, 177+100, white);
        display_digit(blue, 272, 148, H, 8, 16);  
        display_word(blue, 272, 169, M, 8, 16);
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
    
    pthread_t t0;
    if(pthread_create(&t0, NULL, (void *)yanwu, NULL) == -1)
    {
        perror("fail to create pthread t0\n");
        return -1;
    }
    pthread_detach(t0);
    gy_39();

    munmap(p, 480*800*4);
    close(lcd_fd);
    return 0;
}
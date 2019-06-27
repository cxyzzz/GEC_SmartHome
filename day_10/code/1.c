#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

//6818串口所对应的文件名
#define COM2 "/dev/ttySAC1"
#define COM3 "/dev/ttySAC2"
#define COM4 "/dev/ttySAC3"

/*
file:串口所对应的文件名
peed:波特率,是9600
返回值是初始化之后的串口文件的文件描述符
*/
int serial_init(char *file,int peed)
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
	switch(peed)
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

void yanwu()
{
	//1,初始化串口
	int fd = serial_init(COM2,9600);
	if(fd == -1)
    {
        perror("serial_init error");
        return -1;
    }
	
	int fd1 = open("/dev/beep",O_RDWR);
	if(fd1 == -1)
    {
        perror("open beep error");
        return -1;
    }
	
	//2,进行通信
	while(1)
	{
		char w_buf[9] = {0xFF,0x01,0x86,0x00,0,0,0,0,0x79};
		write(fd,w_buf,9);//开发板发送请求给传感器
		usleep(1000);//延时1000微秒，给传感器一些反应的时间
		
		char r_buf[9];
		int r = read(fd,r_buf,9);//接收传感器的返回信息
		if(r == 9 && r_buf[0] == 0xff && r_buf[1] == 0x86)
		{
			int n = r_buf[2]<<8 | r_buf[3];
			printf("n=%d\n",n);
			if(n > 3)
			{
				ioctl(fd1,0,1);
				sleep(3);
				ioctl(fd1,1,1);
				printf("111\n");
			}
			else
			{
				ioctl(fd1,1,1);
				printf("222\n");
			}

		}
		sleep(1);
	}

	close(fd);
	close(fd1);
}

void  GY39()
{
	
	while(1)
	{
		;//
	}
	
}

int main()
{
	yanwu();
	GY39();
	return 0;
	
}

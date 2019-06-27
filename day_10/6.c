	#include<stdio.h>
	#include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
	#include <sys/mman.h> 
	#include "1.h"
	int *p;
	void display(int color,int x,int y)
	{
		*(p+800*x+y)=color;
	}
	void math(int g,char name[],int x1,int y1)//16*16
	{
		int fd;
		fd=open("/dev/fb0",O_RDWR);
		if (fd==-1)
		{
			perror("open 1 erroe\n");
		}
		p=mmap(NULL,480*800*4,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
		int i,j;
		for(i=0;i<32;i++)
		{
			for(j=0;j<8;j++)
			{
				if(name[i]>>(7-j) & 1 == 1)
				{
					display(g,i/2+x1,(i%2)*8+j+y1);
				}
			}
			
		}
		munmap(p,480*800*4);
		close (fd);
		
		
	}
	void ma(int g,char name[],int x1,int y1)//48*48
	{
		int fd;
		fd=open("/dev/fb0",O_RDWR);
		if (fd==-1)
		{
			perror("open 1 erroe\n");
		}
		p=mmap(NULL,480*800*4,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
		int i,j;
		for(i=0;i<288;i++)
		{
			for(j=0;j<8;j++)
			{
				if(name[i]>>(7-j) & 1 == 1)
				{
					display(g,i/6+x1,(i%6)*8+j+y1);
				}
			}
			
		}
		munmap(p,480*800*4);
		close (fd);
		
		
	}

	int main ()
	{
	math(0x0a0a0a,chen,125,16);//名字
	math(0x0a0a0a,ton,125,32);
	math(0x0a0a0a,xin,125,48);
	//math(0x0a0a0a,maohao,125,64);
	
	math(0x0a0a0a,tong1,145,32);//班级
	math(0x0a0a0a,yi,145,48);
	math(0x0a0a0a,maohao,145,64);
	
	math(0x0000ee,kkk[2],145,80);//学号
	math(0x0000ee,kkk[6],145,94);
	
	math(0x0a0a0a,chen,173,16);//名字
	math(0x0a0a0a,xin,173,32);
	math(0x0a0a0a,yu,173,48);
	//math(0x0a0a0a,maohao,173,64);
	
	math(0x0a0a0a,tong1,193,32);//班级
	math(0x0a0a0a,er,193,48);
	math(0x0a0a0a,maohao,193,64);
	
	math(0x0000ee,kkk[1],193,80);//学号
	math(0x0000ee,kkk[8],193,94);
	
	math(0x0a0a0a,mei,221,16);//名字
	math(0x0a0a0a,miao,221,32);
	//math(0x0a0a0a,maohao,221,48);
	
	math(0x0a0a0a,tong1,241,32);//班级
	math(0x0a0a0a,yi,241,48);
	math(0x0a0a0a,maohao,241,64);
	
	math(0x0000ee,kkk[4],241,80);//学号
	math(0x0000ee,kkk[3],241,94);
	
	ma(0x0a0a0a,zhi,25,216);//智能家居系统
	ma(0x0a0a0a,neng,25,280);
	ma(0x0a0a0a,jia,25,344);
	ma(0x0a0a0a,ju,25,408);
	ma(0x0a0a0a,xi,25,472);
	ma(0x0a0a0a,tong,25,536);

	return 0;
	}
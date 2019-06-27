#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main()
{       
    int lcd_fd;
    lcd_fd = open("/dev/fb0",O_RDWR);
    
    if(lcd_fd >= 0)
    {
        printf("open lcd ok!\n");
    }
    else{
        perror("open lcd error!\n");
        return 0;
    }
    
    int * p = mmap(NULL, 480 * 800 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, 0); 
    
    int red = 0x00FF0000, blue = 0x000000FF, black = 0x00000000;

    int i,j;
    for(i = 0; i < 480; i++)
    {
        for(j = 0; j < 800; j++)
        {
        *(p + i * 800 = j ) = blue;
        }
    } 
    
    munmap(p,  480 * 800 * 4);
    
    int ret;
    ret = close(lcd_fd);
    if(ret == 0)
    {
        printf("close lcd ok!\n");
    }
    else{
        perror("close lcd error!\n");
    }
    
    return 0;
}
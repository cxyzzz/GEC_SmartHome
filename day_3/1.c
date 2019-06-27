#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main()
{
    int lcd_fd;
    lcd_fd = open("/dev/fb0", O_RDWR);
    if(lcd_fd == -1)
    {
        perror("open /dev/fb0 error!\n");
    }
    
    int blue = 0x000000FF, bg = 0x00FFFFFF;
    int * p = mmap(NULL, 480*800*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    
    int i, j;
    for(i = 0; i < 480; i++)
    {
        for(j = 0; j < 800; j++)
        {
            if(i > 100 || j > 100)
            {
                *(p + i * 800 + j) = bg;
            }
            else
            {
                *(p + i * 800 + j) = blue;
            }
        }
    }
    
    close(lcd_fd);
    
    return 0;
}

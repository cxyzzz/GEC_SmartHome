#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

void j(int x, int y, int a, int b, int color, int * p)
{
    int i, j;
    int bg = 0x00FFFFFF;

    for(i = 0; i < 480; i++)
    {
        for(j = 0; j < 800; j++)
        {
            if(i < y + b && i >= y && j < x + a && j >= x)
            {
                *(p + i * 800 + j) = color;
            }
            else
            {
                *(p + i * 800 + j) = bg;
            }
        }
    }

}

int main()
{
    int lcd_fd;
    int x, y, a, b, color;
    
    printf("input x, y, a, b, color: \n");
    scanf("%d%d%d%d%d", &x, &y, &a, &b, &color);
    
    lcd_fd = open("/dev/fb0", O_RDWR);
    if(lcd_fd == -1)
    {
        perror("open /dev/fb0 error!\n");
    }
    
    int * p = mmap(NULL, 480*800*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    j(x, y, a, b, color, p);
    
    munmap(p, 480*800*4);
    close(lcd_fd);
    
    return 0;
}

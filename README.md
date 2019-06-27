# GEC_smart_home

GEC 智能家居生产实习。使用的设备为 GEC6818 arm 开发板，开发环境为 Ubuntu 14.04

## [0x01 复习 C 语言基础](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_1)

1. 求两个数的最大值
2. 判断奇偶
3. 判断素数
4. 使用函数交换两个数的值
    - 指针的使用

## [0x02 I/O 练习](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_2)

1. 判断是否为闰年
2. 判断是否为水仙花数
3. 打开一个文件。输出其所有内容
4. 交换两个文件的内容
5. 将屏幕显示为某一颜色
    - 将文件映射到内存，加快读写速度。

## [0x03 在屏幕绘制指定图案](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_3)

1. 画一个矩形
2. 画一个圆
3. 在任意位置画一个任意长度的矩形

## [0x04 在屏幕上显示字符](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_4)

1. 在屏幕上显示自己的名字和学号
2. 任意输入一个数字，在屏幕上显示

## [0x05 在屏幕上显示位图](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_5)

1. 输入一张图片名，在屏幕上显示

BMP 文件格式：
![bmp文件格式.png](https://i.loli.net/2019/06/27/5d14ba5dd16d076060.png)

## [0x06 读取触摸屏数据](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_6)

1. 读取触摸屏数据，获取触摸点坐标

## [0x07 电子相册](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_7)

1. 在屏幕上显示一张图，可以通过滑动屏幕切换图片

## 0x08 休息

## [0x09 串口编程](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_9)

1. 读取串口连接的烟雾传感器数据,当超过一定值时报警
2. 读取 GY-39 传感器数据
GE6818 arm 开发板蜂鸣器文件 /dev/beep ，通过 ioctl 函数控制。

## [0xA 多线程 pthread 的使用](https://github.com/cxyzzz/GEC_smart_home/tree/master/day_10)

1. 创建线程 pthread_create
2. 等待线程结束 pthread_join

参考资料：

1. [Linux中pthread线程使用详解 https://blog.csdn.net/joysonqin/article/details/70237422](https://blog.csdn.net/joysonqin/article/details/70237422)

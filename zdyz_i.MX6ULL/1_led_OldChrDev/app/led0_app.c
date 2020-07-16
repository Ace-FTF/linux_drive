/* 版权声明
 * 功能     ：
 * 作者     ：lsy
 * 文件路径 : /home/lsy/practice/arm/openedv/drive/led/app/led0_app.c
 * 创建时间 ：2020/06/18 19:16
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define LED_NAME  "/dev/led0" /* 设备名 */

int main(int argc, char *argv[])
{
    unsigned char para[3];
    int fd;
    int ret;

    if ((argc == 2) &&
        (strncmp("./led", argv[0], (strlen("./ledx") - 1)) == 0) &&
        ((strncmp("on", argv[1], strlen("on")) == 0) || (strncmp("off", argv[1], strlen("off")) == 0))) {
        /* Don't do anyting. */ 
    } else {
        printf("Usage :\r\n");
        printf("    ./ledx [on/off]\r\n");
        return (-1);
    }

    fd = open(LED_NAME, O_RDWR);
    if (fd < 0) {
        exit(0); 
    }

    ret = write(fd, argv[1], sizeof(argv[1]));
    if (ret < 0) {
        close(fd);
        exit(0); 
    }
    
    close(fd);
    return 0;
}


/* 版权声明
 * 功能     ：
 * 作者     ：lsy
 * 文件路径 : /mnt/hgfs/github/linux_drive/zdyz_i.MX6ULL/2_led_dts/app/led0_dts_app.c
 * 创建时间 ：2020/07/24 10:33
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define LED_DEV_NAME "/dev/led0_dts"

int main(int argc, char *argv[]) {
    int fd;
    int ret;

    if (argc != 2) {
        printf("Usage :\r\n");
        printf("    ./LedDevName on/off\r\n");
        return (-1); 
    } else {
        if ((strncmp(argv[1], "on", strlen("on")) != 0) && 
            (strncmp(argv[1], "off", strlen("off")) != 0)) {
            printf("Usage :\r\n");
            printf("    ./LedDevName on/off\r\n");
            return (-1); 
        }
    }

    fd = open(LED_DEV_NAME, O_RDWR);
    if (fd < 0) {
        printf("Open file failed!\r\n"); 
        exit(0);
    }

    ret = write(fd, argv[1], strlen(argv[1]));
    if (ret < 0) {
        printf("Write file failed!\r\n"); 
        close(fd);
        exit(0);
    }

    close(fd);

    return 0;
}


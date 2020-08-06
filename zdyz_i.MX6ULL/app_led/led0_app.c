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

#define PARA_TOTAL_NUM    3
#define PARA_FIRST        0
#define PARA_SECOND       1
#define PARA_THIRD        2

#define LED_ON            "on"
#define LED_OFF           "off"

#define OK  0
#define NOK 1
#define ERR (-1)

/*
 * 命令格式：
 *     ./led0_app [/dev/dev_name] [on/off]
 *
 * 第一个参数：led测试app名字
 * 第二个参数：驱动名字，路径为/dev/xxx,由用户传入
 * 第三个参数：led状态，由用户传入
 *
 */
int main(int argc, char *argv[]) {
    int fd;
    int ret;

    if (argc != PARA_TOTAL_NUM) {
        printf("Usage :\r\n");
        printf("    ./led0_app [/dev/dev_name] [on/off]\r\n");
        return ERR;
    }

    if ((strncmp(argv[PARA_THIRD], LED_ON, strlen(LED_ON)) != 0) &&
        (strncmp(argv[PARA_THIRD], LED_OFF, strlen(LED_OFF)) != 0)) {
        printf("Usage :\r\n");
        printf("    ./led0_app [/dev/dev_name] [on/off]\r\n");
        return ERR;
    }

    fd = open(argv[PARA_SECOND], O_RDWR);
    if (fd < 0) {
        printf("Open file failed!\r\n");
        exit(0);
    }

    ret = write(fd, argv[PARA_THIRD], strlen(argv[PARA_THIRD]));
    if (ret < 0) {
        printf("Write file failed!\r\n");
        close(fd);
        exit(0);
    }

    close(fd);

    return 0;
}


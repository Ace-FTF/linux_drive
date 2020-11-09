#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <unistd.h>
#include <errno.h>

/* 功能：按键测试app
 * 测试命令：./app_key /dev/key
 */


/* 定义按键值 */
#define KEY0VALUE     0XF0
#define INVAKEY       0X00

#define CMD_TEST_KEY  "./app_key /dev/key"

int main(int argc, char *argv[])
{
    int fd, ret, cnt;
    char *filename;
    int keyvalue;

    if(argc != 2){
        printf("Usage:\r\n");
        printf("    %s\r\n", CMD_TEST_KEY);
        return -1;
    }

    filename = argv[1];
    printf("argv[1] = %s\r\n", filename);

    /* 打开key驱动 */
    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("file %s open failed!\r\n", argv[1]);
        printf("errno = %d\r\n", errno);
        return -1;
    }
    printf("Open dev success!\r\n");

    /* 循环读取按键值数据！ */
    cnt = 0;
    while(1) {
        usleep(1000); //1ms

        read(fd, &keyvalue, sizeof(keyvalue));
        if (keyvalue == KEY0VALUE) {    /* KEY0 */
            printf("KEY0 Press, value = %#X\r\n", keyvalue);    /* 按下 */

            cnt++;
            if (cnt >= 10) { //按键10次跳出循环
                printf("Time up. quit app.\r\n");
                break;
            }
        }
    }

    ret= close(fd); /* 关闭文件 */
    if(ret < 0){
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}

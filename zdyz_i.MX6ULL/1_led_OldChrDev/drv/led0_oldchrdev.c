/*
 * 电路图LED信息：
 *     LED标号     ：LED0
 *     电路网络标号：GPIO3
 *     gpio引脚序号：GPIO1_IO03
 *
 * 驱动信息：
 *     驱动名称    ：/dev/led0
 *     设备号      ：200
 */

#include <linux/string.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LED_MAJOR 200              /* 主设备号 */
#define LED_NAME  "led0_oldchrdev" /* 设备名 */

/* 寄存器物理地址 */
#define CCM_CCGR1_BASE         0x020C406C  /* GPIO时钟使能寄存器 */
#define SW_MUX_GPIO1_IO03_BASE 0x020E0068  /* GPIO复用功能配置寄存器 */
#define SW_PAD_GPIO1_IO03_BASE 0x020E02F4  /* GPIO特性配置寄存器，如：速度、压摆率、配置驱动能力的上下拉电阻大小等 */
#define GPIO1_GDIR_BASE        0x0209C004  /* GPIO方向寄存器 */
#define GPIO1_DR_BASE          0x0209C000  /* GPIO数据寄存器 */

/* 寄存器映射虚拟地址 */
volatile unsigned long *IMX6U_CCM_CCGR1 = NULL;
volatile unsigned long *SW_MUX_GPIO1_IO03 = NULL;
volatile unsigned long *SW_PAD_GPIO1_IO03 = NULL;
volatile unsigned long *GPIO1_GDIR = NULL;
volatile unsigned long *GPIO1_DR = NULL;

/*****************************    分割线：驱动功能函数    *****************************/
/*
 * 功能：led写高电平or低电平
 * 用户层发送命令:./led on/off,即参数user_buff只能是on或off
 */
ssize_t led_write(struct file *file, const char __user *user_buff, size_t cnt, loff_t *loff)
{
    int value;
    unsigned char data_buff[100];

    memset(data_buff, 0, sizeof(data_buff));
    value = copy_from_user(data_buff, user_buff, cnt);
    if (value < 0) {
        printk("copy_from_user failed.\r\n"); 
        return (-EFAULT);
    }

    if (strncmp(data_buff, "on", strlen("on")) == 0) {
        printk("User input : led on.\r\n");
        *GPIO1_DR = ((*GPIO1_DR) & ~(1 << 3));    /* 寄存器写入0为on */
    } else if (strncmp(data_buff, "off", strlen("off")) == 0) {
        printk("User input : led off.\r\n");
        *GPIO1_DR = ((*GPIO1_DR) | (1 << 3));     /* 寄存器写入1为off */
    } else {
        printk("The level must be 0 or 1.\r\n"); 
        return (-EFAULT);
    }

    return 0;
}

/* 函数指针结构体 */
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
};

/*****************************    分割线：驱动入口    *****************************/
/* led驱动入口函数 */
static int __init led_init(void)
{
    int temp;

    /* 初始化led对应GPIO寄存器 */
    /* 1 寄存器地址映射 */
    IMX6U_CCM_CCGR1   = (volatile unsigned long *)ioremap(CCM_CCGR1_BASE, 32);
    SW_MUX_GPIO1_IO03 = (volatile unsigned long *)ioremap(SW_MUX_GPIO1_IO03_BASE, 32);
    SW_PAD_GPIO1_IO03 = (volatile unsigned long *)ioremap(SW_PAD_GPIO1_IO03_BASE, 32);
    GPIO1_GDIR        = (volatile unsigned long *)ioremap(GPIO1_GDIR_BASE, 32);
    GPIO1_DR          = (volatile unsigned long *)ioremap(GPIO1_DR_BASE, 32);

    /* 2 使能时钟 */
    *IMX6U_CCM_CCGR1 = ((*IMX6U_CCM_CCGR1) | (0b11 << 26)); /* gpio1 clock为：26、27bit */

    /* 3 设置复用功能：复用为GPIO功能 */
    *SW_MUX_GPIO1_IO03 = 0b0101;

    /* 4 设置GPIO速率、上拉等特性 */
    *SW_PAD_GPIO1_IO03 = 0x10B0; /* 0b0001000010110000 */

    /* 5 设置GPIO方向：输出 */
    *GPIO1_GDIR = ((*GPIO1_GDIR) | (1 << 3));

    /* 6 默认关闭led */
    *GPIO1_DR = ((*GPIO1_DR) | (1 << 3));

    /* 7 注册字符设备驱动 */
    temp = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
    if (temp < 0) {
        printk("Register led chrdev failed!\r\n"); 
        return -EIO;
    }

    return 0;
}

/* led驱动出口函数 */
static void __exit led_exit(void)
{
    /* 1 寄存器地址解除映射 */
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_GDIR);
    iounmap(GPIO1_DR);

    /* 2 注销字符设备驱动 */
    unregister_chrdev(LED_MAJOR, LED_NAME);
}

module_init(led_init);
module_exit(led_exit);

/*****************************    分割线：声明    *****************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lsy");


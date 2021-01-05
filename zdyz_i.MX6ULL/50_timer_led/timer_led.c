#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/*********    宏               *******************************************/

/*********    设备结构体       *******************************************/
struct timer_led_dev {
    dev_t devid; /* 设备号 */
    int major;
    int minor;

    struct cdev cdev;
    struct class *class;
    struct device *device;

    struct device_node *nd;  /* 设备节点 */

    int led_gpio;            /* led gpio序号 */

    struct timer_list timer; /* 定时器 */
    int time_period;         /* 定时周期 */

    spinlock_t lock;         /* 自旋锁 */
};

struct timer_led_dev timer_led_dev; /* 定时器设备 */

/*********    功能函数         *******************************************/
/*
 * led初始化函数，获取设备树中的gpio引脚，获取GPIO资源，初始化GPIO
 */
static int led_init(void)
{

}

/*********    fops函数集      *******************************************/
static int timer_led_open()
{

}

static long timer_led_unlocked_ioctl()
{

}

static struct file_operations timer_led_fops = {
    .owner = THIS_MODULE,
    .open = timer_led_open,
    .unlocked_ioctl = timer_led_unlocked_ioctl,
};

/*
 * 定时器回调函数
 */
void timer_led_function()
{

}


/*********    驱动函数框架    *******************************************/
/*
 * 驱动入口函数
 */
static int __init timer_led_init(void)
{

    return 0;
}

/*
 * 驱动出口函数
 */
static void __exit timer_led_exit(void)
{

}

/*
 * 模块注册
 */
module_init(timer_led_init);
module_exit(timer_led_exit);

/*********   声明  *******************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lsy");


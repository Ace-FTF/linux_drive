/*
 * 电路图LED信息：
 *     LED标号     ：LED0
 *     电路网络标号：GPIO3
 *     gpio引脚序号：GPIO1_IO03
 *
 * 驱动信息：
 *     驱动名称    ：/dev/led0_newchrdev
 *     设备号      ：系统自动分配
 */

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

#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LED_DEV_NAME "led0_newchrdev"
#define LED_DEV_CNT  1

/* 寄存器物理地址 */
#define CCM_CCGR1_BASE         0x020C406C  /* GPIO时钟使能寄存器 */
#define SW_MUX_GPIO1_IO03_BASE 0x020E0068  /* GPIO复用功能配置寄存器 */
#define SW_PAD_GPIO1_IO03_BASE 0x020E02F4  /* GPIO特性配置寄存器，如：速度、压摆率、配置驱动能力的上下拉电阻大小等 */
#define GPIO1_GDIR_BASE        0x0209C004  /* GPIO方向寄存器 */
#define GPIO1_DR_BASE          0x0209C000  /* GPIO数据寄存器 */

/* 寄存器映射虚拟地址 */
volatile unsigned long *imx6u_ccm_ccgr1 = NULL;
volatile unsigned long *sw_mux_gpio1_io03 = NULL;
volatile unsigned long *sw_pad_gpio1_io03 = NULL;
volatile unsigned long *gpio1_gdir = NULL;
volatile unsigned long *gpio1_dr = NULL;

/*************************             创建设备             ***********************************/
/* 抽象设备结构体 */
typedef struct {
    dev_t devid;            /* 设备号 */
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    struct cdev cdev;       /* cdev */
    struct class *class;    /* 类 */          //----->疑问：这里为什么要定义成指针
    struct device *device;  /* 设备 */
} led_device_struct;
led_device_struct led; /* 实例化设备 */

/*************************  功能函数和file_operation结构体  ***********************************/
#define LED_ON  1 /* 寄存器写入0为on */
#define LED_OFF 0 /* 寄存器写入1为off */

/* led亮灭操作函数 */
void led_set(int led_status) {
    int val = 0;
    if (led_status == LED_ON) {
        /* 寄存器写入0为on */
        val = readl(gpio1_dr);
        val &= ~(1 << 3); 
        writel(val, gpio1_dr);
    } else if (led_status == LED_OFF){
        /* 寄存器写入1为off */
        val = readl(gpio1_dr);
        val |= (1 << 3); 
        writel(val, gpio1_dr);
    } else {
        printk("Set led status error!\r\n"); 
    }
}

/* 功能函数1：open */
static int led_open(struct inode *inode, struct file *file) {

    return 0;
}

/* 功能函数2：write */
static ssize_t led_write(struct file *file, const char __user *user_buff, size_t cnt, loff_t *loff) {
    int val;
    unsigned char data_buff[100];

    memset(data_buff, 0, sizeof(data_buff));
    val = copy_from_user(data_buff, user_buff, cnt);
    if (val < 0) {
        printk("copy_from_user failed.\r\n"); 
        return (-EFAULT);
    }

    if (strncmp(data_buff, "on", strlen("on")) == 0) {
        led_set(LED_ON);
    } else if (strncmp(data_buff, "off", strlen("off")) == 0) {
        led_set(LED_OFF);
    } else {
        printk("The level must be 0 or 1.\r\n"); 
        return (-EFAULT);
    }

    return 0;
}

/* file_operations结构体 */
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
};

/*************************    驱动入口/出口函数    ***********************************/
static int __init led_init(void) {
    int val;

    /* 一、寄存器虚拟映射 */
    imx6u_ccm_ccgr1   = ioremap(CCM_CCGR1_BASE, 32);
    sw_mux_gpio1_io03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 32);
    sw_pad_gpio1_io03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 32);
    gpio1_gdir        = ioremap(GPIO1_GDIR_BASE, 32);
    gpio1_dr          = ioremap(GPIO1_DR_BASE, 32);

    /* 二、gpio初始化为输出模式 */
    val = readl(imx6u_ccm_ccgr1);
    val &= ~(0b11 << 26);   /* 26,27bit控制GPIO1的时钟，先清除原来的值 */
    val |= (0b11 << 26);
    writel(val, imx6u_ccm_ccgr1);

    writel(0b0101, sw_mux_gpio1_io03); /* 复用为IO功能 */

    writel(0b0001000010110000, sw_pad_gpio1_io03); /* 0001 0000 1011 0000，控制上下拉、速率等 */

    val = readl(gpio1_gdir); /* 设置输出模式 */
    val |= (1 << 3);         /* bit3位直接控制GPIO1_IO03的模式 */
    writel(val, gpio1_gdir);
    
    val = readl(gpio1_dr);   /* 设置默认输出高电平 */
    val |= (1 << 3);
    writel(val, gpio1_dr);

    /* 三、创建设备号 */
    led.devid = 0;
    led.major = 0;
    led.minor = 0;
    if (led.major != 0) {   /* 定义了设备号 */
        led.devid = MKDEV(led.major, 0);
        register_chrdev_region(led.devid, LED_DEV_CNT, LED_DEV_NAME);
    } else {                /* 未定义设备号 */
        alloc_chrdev_region(&led.devid, 0, LED_DEV_CNT, LED_DEV_NAME);
        led.major = MAJOR(led.devid);
        led.minor = MINOR(led.devid);
    }
    printk("The new led device major = %d, minor = %d\r\n", led.major, led.minor);

    /* 四、初始化cdev结构体 */
    led.cdev.owner = THIS_MODULE;
    cdev_init(&led.cdev, &led_fops);
    cdev_add(&led.cdev, led.devid, LED_DEV_CNT);
    
    /* 五、创建类 class */
    led.class = class_create(THIS_MODULE, LED_DEV_NAME);
    if (IS_ERR(led.class)) {
        return PTR_ERR(led.class); 
    }

    /* 六、创建设备 device */
    led.device = device_create(led.class, NULL, led.devid, NULL, LED_DEV_NAME);
    if (IS_ERR(led.device)) {
        return PTR_ERR(led.device); 
    }

    return 0;
}

static void __exit led_exit(void) {
    /* 一、寄存器取消映射 */
    iounmap(imx6u_ccm_ccgr1);
    iounmap(sw_mux_gpio1_io03);
    iounmap(sw_pad_gpio1_io03);
    iounmap(gpio1_gdir);
    iounmap(gpio1_dr);

    /* 二、删除cdev和设备号（注意删除顺序） */
    cdev_del(&led.cdev);                              /* 删除cdev结构体 */
    unregister_chrdev_region(led.devid, LED_DEV_CNT); /* 释放设备号 */

    /* 三、摧毁设备和类（注意删除顺序） */
    device_destroy(led.class, led.devid); /* 摧毁设备 */
    class_destroy(led.class);/* 摧毁类 */
}

module_init(led_init);
module_exit(led_exit);

/*************************          声明           ***********************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lsy");


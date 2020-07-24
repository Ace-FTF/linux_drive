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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LED0_DTS_NODE_PATH "/lsy_led0"
#define LED0_DTS_DEV_NAME  "led0_dts"
#define LED0_DTS_DEV_CNT   1

#define LED_ON      0
#define LED_OFF     1
#define LED_CMD_LEN 16

/********************     定义寄存器指针    ****************************/
static void __iomem *CCM_CCGR1_BASE;
static void __iomem *SW_MUX_GPIO1_IO03_BASE;
static void __iomem *SW_PAD_GPIO1_IO03_BASE;
static void __iomem *GPIO1_GDIR_BASE;
static void __iomem *GPIO1_DR_BASE;

/********************       设备结构体      ****************************/
struct led_dts_dev {
    dev_t devid;
    int   major;
    int   minor;

    struct cdev        cdev;
    struct class       *class;
    struct device      *device;
    struct device_node *node;
};
struct led_dts_dev led0_dts_dev; /* 定义led设备 */

/********************      led操作函数       ****************************/
void led0_set_status(int status)
{
    u32 reg_value;

    reg_value = readl(GPIO1_DR_BASE);

    if (status == 0) {
        reg_value &= ~(1 << 3);  /* 低电平，灯亮 */
    } else if (status == 1) {
        reg_value |= (1 << 3);   /* 高电平，灯灭 */
    } else {
        printk("led status err!\r\n"); 
        return;
    }

    printk("reg_value of GPIO1_DR_BASE : %#x\r\n", reg_value);
    writel(reg_value, GPIO1_DR_BASE);
}

/******************** file_operation结构体  ****************************/
static int led_open(struct inode *inode, struct file *file)
{
    file->private_data = &led0_dts_dev; /* 设置私有数据 */
    return 0;
}

static ssize_t led_read(struct file *file, char __user *user_buf, size_t cnt, loff_t *offset)
{
    return 0;
}

static ssize_t led_write(struct file *file, const char __user *user_buf, size_t cnt, loff_t *offset)
{
    int ret;
    unsigned char data_buf[LED_CMD_LEN]; 

    if (cnt > sizeof(data_buf) - 1) {
        printk("Cmd is too long!\r\n");
        goto err;
    }

    /* 获取用户层数据 */
    memset(data_buf, 0, sizeof(data_buf));
    ret = copy_from_user(data_buf, user_buf, cnt);
    if (ret < 0) {
        printk("copy_from_user error!\r\n");
        goto err;
    }
    printk("user data : %s\r\n", data_buf);
    
    /* 点灯 */
    if (strncmp(data_buf, "on", strlen("on")) == 0) {          /* led on */
        led0_set_status(LED_ON); 
        printk("led on.\r\n");
    } else if (strncmp(data_buf, "off", strlen("off")) == 0) { /* led off */
        led0_set_status(LED_OFF); 
        printk("led off.\r\n");
    } else {
        printk("Cmd input error.\r\n"); 
        goto err;
    }

    return 0;

err:
    return (-EFAULT);
}

static int led_release(struct inode *inode, struct file *file)
{
    return 0;
}

static struct file_operations led0_dts_fops = {
    .owner   = THIS_MODULE,
    .open    = led_open,
    .read    = led_read,
    .write   = led_write,
    .release = led_release,
};

/********************    驱动出入口函数     ****************************/
/* 入口函数 */
static int __init led_dts_init(void)
{
    u32 reg_value;
    int i;
    u32 reg_data[14];
    const char *str = NULL;
    int ret;
    int err;
    struct property *prop;

    /* 一、获取设备树节点中的属性数据 */
    /* 1. 根据节点路径查找节点，属性信息保存在node结构体中 */
    led0_dts_dev.node = of_find_node_by_path(LED0_DTS_NODE_PATH);
    if (led0_dts_dev.node == NULL) {
        printk("%s can not found!\r\n", LED0_DTS_NODE_PATH);
        err =  -EINVAL;
        goto err;
    } else {
        printk("Node : %s founded!\r\n", LED0_DTS_NODE_PATH);
    }

    /* 2. 练习：获取compatable，status，reg三个属性内容，并打印 */
    prop = of_find_property(led0_dts_dev.node, "compatible", NULL);
    if (!prop) {
        err = -EINVAL;
        goto err; 
    } else {
        printk("compatible = %s\r\n", (char *)(prop->value)); 
    }

    ret = of_property_read_string(led0_dts_dev.node, "status", &str);
    if (ret < 0) {
        printk("Read status failed!\r\n");
        err = -EINVAL;
        goto err; 
    } else {
        printk("status = %s\r\n", str); 
    }

    ret = of_property_read_u32_array(led0_dts_dev.node, "reg", reg_data, 10);
    if (ret < 0) {
        printk("Read reg data failed!\r\n");
        err = -EINVAL;
        goto err; 
    } else {
        printk("reg_data[] :");
        for (i = 0; i < 10; i++) {
            if (i%2 == 0) {
                printk("\r\n");
                printk("    ");
            }
            printk("0x%.8X ", reg_data[i]); 
        }
        printk("\r\n");
    }

    /* 二、led初始化 */
    /* 1. 寄存器地址映射 */
    CCM_CCGR1_BASE         = of_iomap(led0_dts_dev.node, 0);
    SW_MUX_GPIO1_IO03_BASE = of_iomap(led0_dts_dev.node, 1);
    SW_PAD_GPIO1_IO03_BASE = of_iomap(led0_dts_dev.node, 2);
    GPIO1_DR_BASE          = of_iomap(led0_dts_dev.node, 3);
    GPIO1_GDIR_BASE        = of_iomap(led0_dts_dev.node, 4);

    /* 2. 使能GPIO时钟 */
    reg_value = readl(CCM_CCGR1_BASE);
    reg_value |= (0b11 << 26);
    writel(reg_value, CCM_CCGR1_BASE);

    /* 3. 设置GPIO属性 */
    writel(0x5, SW_MUX_GPIO1_IO03_BASE);    /* 0101，设置GPIO复用功能，复用为GPIO1_IO03 */
    writel(0x10B0, SW_PAD_GPIO1_IO03_BASE); /* 0001 0000 1011 0001，设置GPIO速率等属性 */

    /* 4. 设置GPIO方向寄存器为输出 */
    reg_value = readl(GPIO1_GDIR_BASE);
    reg_value |= (1 << 3);
    writel(reg_value, GPIO1_GDIR_BASE);

    /* 5. 设置GPIO数据寄存器，给定默认值，设置gpio默认输出状态 */
    reg_value = readl(GPIO1_DR_BASE);
    reg_value |= (1 << 3); /* 默认输出高电平，灯灭 */
    writel(reg_value, GPIO1_DR_BASE);

    /* 三、新字符设备驱动框架 */
    /* 1. 申请设备号 */
    if (led0_dts_dev.major != 0) { /* 手动指定了设备号 */
        led0_dts_dev.devid = MKDEV(led0_dts_dev.major, 0);
        err = register_chrdev_region(led0_dts_dev.devid, LED0_DTS_DEV_CNT, LED0_DTS_DEV_NAME);
        if (err < 0) {
            printk("register_chrdev_region failed.\r\n");
            goto err; 
        }
    } else {                       /* 系统自动分配设备号 */
        err = alloc_chrdev_region(&led0_dts_dev.devid, 0, LED0_DTS_DEV_CNT, LED0_DTS_DEV_NAME);
        if (err < 0) {
            printk("alloc_chrdev_region failed. Can't get major number.\r\n");
            goto err;
        }
        led0_dts_dev.major = MAJOR(led0_dts_dev.devid);
        led0_dts_dev.minor = MINOR(led0_dts_dev.devid);
    
    }

    /* 2. 初始化cdev结构体 */
    led0_dts_dev.cdev.owner = THIS_MODULE;
    cdev_init(&led0_dts_dev.cdev, &led0_dts_fops);
    err = cdev_add(&led0_dts_dev.cdev, led0_dts_dev.devid, LED0_DTS_DEV_CNT);
    if (err < 0) {
        printk("cdev_add error!\r\n"); 
        goto err_cdev_add;
    }

    /* 3. 创建类 */
    led0_dts_dev.class = class_create(THIS_MODULE, LED0_DTS_DEV_NAME);
    if (IS_ERR(led0_dts_dev.class)) {
        err = PTR_ERR(led0_dts_dev.class); 
        goto err_class_create;
    }

    /* 4. 创建设备 */
    led0_dts_dev.device = device_create(led0_dts_dev.class, NULL, led0_dts_dev.devid, NULL, LED0_DTS_DEV_NAME);
    if (IS_ERR(led0_dts_dev.device)) {
        err = PTR_ERR(led0_dts_dev.device); 
        goto err_device_create;
    }

    return 0;

err_device_create:
    class_destroy(led0_dts_dev.class);
err_class_create:
    cdev_del(&led0_dts_dev.cdev);
err_cdev_add:
    unregister_chrdev_region(led0_dts_dev.devid, LED0_DTS_DEV_CNT);
err:
    return err;
}

/* 出口函数 */
static void __exit led_dts_exit(void)
{
    /* 一、取消IO映射 */
    iounmap(CCM_CCGR1_BASE); 
    iounmap(SW_MUX_GPIO1_IO03_BASE); 
    iounmap(SW_PAD_GPIO1_IO03_BASE); 
    iounmap(GPIO1_DR_BASE); 
    iounmap(GPIO1_GDIR_BASE); 

    /* 二、新字符设备驱动框架注销 */
    /* 2.1 释放cdev结构体 */
    cdev_del(&led0_dts_dev.cdev);

    /* 2.2 释放设备号 */
    unregister_chrdev_region(led0_dts_dev.devid, LED0_DTS_DEV_CNT);
    
    /* 2.3 摧毁设备 */
    device_destroy(led0_dts_dev.class, led0_dts_dev.devid);

    /* 2.4 摧毁类 */
    class_destroy(led0_dts_dev.class);
}

module_init(led_dts_init);
module_exit(led_dts_exit);

/********************         声明          ****************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lsy");


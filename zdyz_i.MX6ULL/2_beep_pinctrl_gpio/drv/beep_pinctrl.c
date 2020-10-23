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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define BEEP_DTS_DEV_CNT   1
#define BEEP_DTS_DEV_NAME  "beep_pinctrl"

/* 声明结构体 */
struct beep_struct {
    dev_t devid;
    int major;
    int minor;

    struct cdev        cdev;
    struct class       *class;
    struct device      *device;
    struct device_node *node;

    int gpio_num;
};
struct beep_struct beep_dev; /* 定义beep设备结构体 */

static int beep_open(struct inode *inode, struct file *file)
{
    file->private_data = &beep_dev; /* 设置私有数据 */
    return 0;
}

static ssize_t beep_write(struct file *file, const char __user *buffer, size_t count, loff_t  *offt)
{
    char kernel_buff[16];
    struct beep_struct *beep = file->private_data;

    if (count >= sizeof(kernel_buff)) {
        printk("Recv user data is too long.\r\n");
        return -EFAULT;
    }

    printk("kernel log : userbuff = %s\r\n", buffer);

    /* 拷贝用户空间数据到内核空间 */
    memset(kernel_buff, 0, sizeof(kernel_buff));
    if (copy_from_user(kernel_buff, buffer, count) != 0) {
        printk("Error cmd from user!\r\n");
        return -EFAULT;
    }

    /* 解析拷贝的用户空间命令字符串，并执行对应操作 */
    if (strncmp("on", kernel_buff, strlen(kernel_buff)) == 0) {
        gpio_set_value(beep->gpio_num, 0); /* 打开beep */
    } else if (strncmp("off", kernel_buff, strlen(kernel_buff)) == 0) {
        gpio_set_value(beep->gpio_num, 1); /* 关闭beep */
    } else {
        printk("cmd err!\r\n");
        return -EFAULT;
    }

    return 0;
}

/* file_operations 结构体，关联用户空间函数和驱动空间函数 */
static const struct file_operations beep_ops = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .write = beep_write,
};

/* 入口函数 */
static int __init beep_init(void)
{
    int err;

    /* of家族函数：获取节点特性 */
    beep_dev.node = of_find_compatible_node(NULL, NULL, "atkalpha-gpiobeep");
    if (beep_dev.node == 0) {
        printk("of_find_compatible_node failed!\r\n");
        return -ENODEV;
    }

    /* of家族函数：获取gpio_num */
    beep_dev.gpio_num = of_get_named_gpio(beep_dev.node, "beep-gpio", 0);
    if (beep_dev.gpio_num < 0) {
        printk("Can't get gpio num!\r\n");
        return -ENODEV;
    }

    /* gpio子系统API：直接设置gpio电平 */
    err = gpio_direction_output(beep_dev.gpio_num, 1);
    if (err < 0) {
        printk("Set gpio level err!\r\n");
        return -ENODEV;
    }

    /* 申请设备号 */
    if (beep_dev.major != 0) {     /* 已经分配过设备号 */
        beep_dev.devid = MKDEV(beep_dev.major, 0);
        err = register_chrdev_region(beep_dev.devid, BEEP_DTS_DEV_CNT, BEEP_DTS_DEV_NAME);
        if (err) {
            pr_err("Kernel function register_chrdev_region failed with err code %d.\r\n", err);
            goto err_get_devid;
        }
    } else {                       /* 自动分配设备号 */
        err = alloc_chrdev_region(&beep_dev.devid, 0, BEEP_DTS_DEV_CNT, BEEP_DTS_DEV_NAME);
        if (err) {
            pr_err("Kernel function alloc_chrdev_region failed with err code %d.\r\n", err);
            goto err_get_devid;
        }
        beep_dev.major = MAJOR(beep_dev.devid);
        beep_dev.minor = MAJOR(beep_dev.devid);
    }

    /* cdev初始化 */
    beep_dev.cdev.owner = THIS_MODULE;
    cdev_init(&beep_dev.cdev, &beep_ops);
    err = cdev_add(&beep_dev.cdev, beep_dev.devid, BEEP_DTS_DEV_CNT);
    if (err < 0) {
        printk("cdev_add err!\r\n");
        goto err_cdev_add;
    }

    /* 创建类 */
    beep_dev.class = class_create(THIS_MODULE, BEEP_DTS_DEV_NAME);
    if (IS_ERR(beep_dev.class)) {
        err = PTR_ERR(beep_dev.class);
        goto err_class_create;
    }

    /* 创建设备 */
    beep_dev.device = device_create(beep_dev.class, NULL, beep_dev.devid, NULL, BEEP_DTS_DEV_NAME);
    if (IS_ERR(beep_dev.device)) {
        err = PTR_ERR(beep_dev.device);
        goto err_device_create;
    }

    return 0;
err_device_create:
    class_destroy(beep_dev.class);
err_class_create:
    cdev_del(&beep_dev.cdev);
err_cdev_add:
    unregister_chrdev_region(beep_dev.devid, BEEP_DTS_DEV_CNT);
err_get_devid:
    return err;
}

/* 出口函数 */
static void __exit beep_exit(void)
{
    cdev_del(&beep_dev.cdev);
    unregister_chrdev_region(beep_dev.devid, BEEP_DTS_DEV_CNT);

    device_destroy(beep_dev.class, beep_dev.devid);
    class_destroy(beep_dev.class);
}

module_init(beep_init);
module_exit(beep_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lsy");

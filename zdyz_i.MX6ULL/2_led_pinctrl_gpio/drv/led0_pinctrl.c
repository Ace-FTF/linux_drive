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

#define LED_DTS_DEV_CNT   1
#define LED_DTS_DEV_NAME  "led0_pinctrl"
#define LED_DTS_NODE_PATH "/lsy_led0_usePinCtrl"

/* 声明结构体，不需要等号，成员之间分号间隔 */ //--->无等号、分号、末尾分号
struct led_device {
    dev_t devid;
    int   major;
    int   minor;

    struct cdev        cdev;
    struct class       *class;
    struct device      *device;
    struct device_node *node;

    int    gpio_num;
};
struct led_device led_dev; /* 定义led设备结构体 */

static int led_open(struct inode *inode, struct file *file)
{
    file->private_data = &led_dev; /* 设置私有数据 */
    return 0;
}

static ssize_t led_write(struct file *file, const char __user *buffer, size_t count, loff_t *offt)
{
    char kenel_buff[16];
    struct led_device *led = file->private_data;

    if (count >= sizeof(kenel_buff)) {
        return -EFAULT;
    }

    printk("kernel log: userbuff = %s\r\n", buffer);

    memset(kenel_buff, 0, sizeof(kenel_buff));
    if (copy_from_user(kenel_buff, buffer, count) != 0) {
        printk("Error cmd form user!\r\n");
        return -EFAULT;
    }

    if (strncmp("on", kenel_buff, strlen(kenel_buff)) == 0) {
        gpio_set_value(led->gpio_num, 0); /* 打开LED */
    } else if (strncmp("off", kenel_buff, strlen(kenel_buff)) == 0) {
        gpio_set_value(led->gpio_num, 1); /* 关闭LED */
    } else {
        return -EFAULT;
    }

    memset(kenel_buff, 0, sizeof(kenel_buff));

    return 0;
}

/* 创建成员，给成员赋值，需要等号，成员赋值之间逗号分隔 */ //-->有等号、逗号、末尾分号
static const struct file_operations led_ops = {
    .owner = THIS_MODULE,
    .open  = led_open,
    .write = led_write,
};

/* 入口函数 */
static int __init led_init(void)
{
    int err;

    led_dev.node = of_find_compatible_node(NULL, NULL, "lsy-dont-know-why-gpioled");
    if (led_dev.node == 0) {
        return -ENODEV;
    }

    led_dev.gpio_num = of_get_named_gpio(led_dev.node, "led-gpio", 0);
    if (led_dev.gpio_num < 0) {
        printk("Can't get gpio num!\r\n");
        return -ENODEV;
    }

    err = gpio_direction_output(led_dev.gpio_num, 0);
    if (err < 0) {
        printk("Set gpio level err!\r\n");
        return -ENODEV;
    }

    /* 申请设备号 */
    if (led_dev.devid) {
        led_dev.devid = MKDEV(led_dev.major, 0);
        err = register_chrdev_region(led_dev.devid, LED_DTS_DEV_CNT, LED_DTS_DEV_NAME);
        if (err) {
            pr_err("Kernel function register_chrdev_region failed with error code %d\r\n", err);
            goto err_get_devid;
        }
    } else {
        err = alloc_chrdev_region(&led_dev.devid, 0, LED_DTS_DEV_CNT, LED_DTS_DEV_NAME);
        if (err) {
            pr_err("Kernel function alloc_chrdev_region failed with error code %d\r\n", err);
            goto err_get_devid;
        }
        led_dev.major = MAJOR(led_dev.devid);
        led_dev.minor = MINOR(led_dev.devid);
    }

    /* cdev初始化 */
    led_dev.cdev.owner = THIS_MODULE;
    cdev_init(&led_dev.cdev, &led_ops);
    err = cdev_add(&led_dev.cdev, led_dev.devid, LED_DTS_DEV_CNT);
    if (err < 0) {
        printk("cdev_add err!\r\n");
        goto err_cdev_add;
    }

    /* 创建类 */
    led_dev.class = class_create(THIS_MODULE, LED_DTS_DEV_NAME);
    if (IS_ERR(led_dev.class)) {
        err = PTR_ERR(led_dev.class);
        goto err_class_create;
    }

    /* 创建设备 */
    led_dev.device = device_create(led_dev.class, NULL, led_dev.devid, NULL, LED_DTS_DEV_NAME);
    if (IS_ERR(led_dev.device)) {
        err = PTR_ERR(led_dev.device);
        goto err_device_create;
    }

    return 0;

err_device_create:
    class_destroy(led_dev.class);
err_class_create:
    cdev_del(&led_dev.cdev);
err_cdev_add:
    unregister_chrdev_region(led_dev.devid, LED_DTS_DEV_CNT);
err_get_devid:
    return err;
}

/* 出口函数 */
static void __exit led_exit(void)
{
    cdev_del(&led_dev.cdev); /* 删除cdev */
    unregister_chrdev_region(led_dev.devid, LED_DTS_DEV_CNT); /* 释放设备号 */

    device_destroy(led_dev.class, led_dev.devid);
    class_destroy(led_dev.class);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lsy");

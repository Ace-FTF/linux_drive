# 一、驱动框架说明

1. 本demo为linux新字符设备驱动代码框架。支持内核版本号为2.16之后。
2. 驱动框架中包含自动生成设备号、自动生成设备节点，加载驱动之后只需要直接使用led app测试即可
3. 测试app可以使用1_led_OldChrDev工程中的测试app。





# 二、led驱动加载测试步骤

1. 进入u-boot，启动linux内核

   ```shell
   tftp 80800000 zImage
   tftp 83000000 imx6ull-alientek-emmc.dtb
   bootz 80800000 - 83000000
   ```

2. 拷贝led驱动ko文件和led_app文件到开发板（在arm板卡中执行下述命令）

   ```shell
   scp -r lsy@192.168.50.250:/mnt/hgfs/github/linux_drive/zdyz_i.MX6ULL/1_led_NewChrDev/drv/led_new_chr_dev.ko .
   ```
   
3. 加载驱动

   ```shell
   insmod led_new_chr_dev.ko
   ```

5. 测试app

   ```shell
   ./led0_app [on/off]
   ```




# 三、卸载驱动

1. 卸载驱动设备命令：

   ```shell
   rmmod led0.ko
   ```

   


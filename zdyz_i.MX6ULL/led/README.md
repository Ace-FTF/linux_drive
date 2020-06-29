led驱动加载测试步骤：

1. 进入u-boot，启动linux内核

   ```shell
   tftp 80800000 zImage
   tftp 83000000 imx6ull-alientek-emmc.dtb
   bootz 80800000 - 83000000
   ```

2. 拷贝led驱动ko文件和led_app文件到开发板（在arm板卡中执行下述命令）

   ```shell
   scp -r lsy@192.168.50.250:~/practice/arm/zdyz/drive/led/drv/led0.ko .
   scp -r lsy@192.168.50.250:~/practice/arm/zdyz/drive/led/app/led0_app .
   ```

3. 加载驱动

   ```shell
   insmod led0.ko
   ```

4. 创建节点

   ```shell
   mknod /dev/led0.ko c 200 0
   ```

5. 测试app

   ```shell
   ./led0_app [on/off]
   ```

   
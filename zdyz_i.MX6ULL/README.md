正点原子开发板imx6ull，网络文件系统启动命令流程：

进入u-boot，执行如下命令启动网络文件系统：（前提：tftp服务器和nfs服务器已经搭建完成）

```shell
tftp 80800000 zImage
tftp 83000000 imx6ull-alientek-emmc.dtb
bootz 80800000 - 83000000
```


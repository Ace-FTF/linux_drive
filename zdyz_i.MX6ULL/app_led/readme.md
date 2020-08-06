1. app文件释义：
    这是一个通用led测试app，通过将用户在命令行输入的on/off传入内核层，
    从而内核根据字符串on或者off点亮或者关闭led指示灯。

    备注：下述命令中，write函数仅将on/off字符串下传到了内核空间。

2. led测试命令格式：
    ./led0_app [/dev/dev_name] [on/off]

3. 参数含义
    第一个参数：led测试app名字
    第二个参数：驱动名字，路径为/dev/xxx,由用户传入
    第三个参数：led状态，由用户传入

4. 测试步骤
    a.) 加载驱动ko文件到内核，会在/dev/目录下生成驱动文件/dev/xxx
    b.) 执行led测试命令

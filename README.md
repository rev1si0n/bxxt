# BXXT

> 前言: 注意，刷机有风险，请确保你有足够的经验保证手机不会变黑砖。

bxxt 是一个在研究安卓物理机群控过程中搞出来的一个自用小工具。
注意：这个工具只能在模拟器及实际机器的环境运行，它`并不能`运行在你的 linux 主机或者电脑上。

它目前在安卓 6.0 - 安卓 10 部分设备上经过了测试。

目前，它的所有功能都是基于我本人的需求，所以并不确保它适合你。它目前可以处理 boot.img 的解包打包，以及 selinux 临时及动态修改。
你不需要网上大多数文章中那些繁琐的步骤以及脚本。

> 首先，请确保下载了当前目录下的 `bxxt` 并且 push 到手机上且加上了可执行权限

## BOOT.IMG

事实上，你只需要这一条命令
```bash
bxxt boot -i boot.img -o /path/to/output
```
即可把 boot.img 中的大部分东西，变成你想要的样子（它会连内核也帮你解压好）。

同样，打包也非常简单
```bash
bxxt boot -i /path/to/output -o boot_new.img
```

下面，来介绍解包输出的文件及结构。
```bash
$
$ ls /path/to/output
-rw-rw-rw-    1 0        0              738 Mar 27 10:31 METADATA       # 元数据，在此编辑 cmdline 类参数
-rw-------    1 0        0         39450632 Mar 27 10:31 kernel         # 解压好的 linux 内核
-rw-rw-rw-    1 0        0           624384 Mar 27 10:31 kernel.dts-00  # 设备树（文本文件，可以编辑）
-rw-rw-rw-    1 0        0           633469 Mar 27 10:31 kernel.dts-01
-rw-rw-rw-    1 0        0           624449 Mar 27 10:31 kernel.dts-02
drwxr-xr-x    9 0        0             4096 Mar 27 10:31 ramdisk        # ramdisk 文件夹
-rw-r--r--    1 0        0                0 Mar 26 18:02 recovery_dtbo  # ...
-rw-r--r--    1 0        0                0 Mar 26 18:02 second         # ...
$
$ ls /path/to/output/ramdisk
drwxr-xr-x    2 0        0             4096 Jan  1  1970 apex
drwxr-xr-x    2 0        0             4096 Jan  1  1970 debug_ramdisk
drwxr-xr-x    2 0        0             4096 Jan  1  1970 dev
-rwxr-x---    1 0        0          1891328 Jan  1  1970 init
drwxr-xr-x    2 0        0             4096 Jan  1  1970 mnt
drwxr-xr-x    2 0        0             4096 Jan  1  1970 proc
drwxr-xr-x    2 0        0             4096 Jan  1  1970 sys
-rw-r--r--    1 0        0              524 Jan  1  1970 verity_key
$
$ cat /path/to/output/METADATA
bxxt.kernel_addr=8000                 # 可编辑
bxxt.ramdisk_addr=1000000             # 可编辑
bxxt.second_addr=f00000               # 可编辑
bxxt.tags_addr=100                    # 可编辑
bxxt.dtb_addr=0                       # 可编辑
bxxt.recovery_dtbo_offset=0           # 可编辑
bxxt.name=                            # 可编辑
bxxt.cmdline=androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x37 # 可编辑
bxxt.extra_cmdline=                   # 可编辑
bxxt.kernel_compression=1             # ! 不可编辑
bxxt.header_version=0                 # ! 不可编辑
bxxt.os_version=14000144              # 可编辑
bxxt.header_size=0                    # ! 不可编辑
bxxt.page_size=1000                   # 可编辑
bxxt.kernel_size=14890c7              # ! 不可编辑
bxxt.ramdisk_size=c5302               # ! 不可编辑
bxxt.second_size=0                    # ! 不可编辑
bxxt.recovery_dtbo_size=0             # ! 不可编辑
bxxt.dtb_size=0                       # ! 不可编辑
$
$ head /path/to/output/kernel.dts-00
/dts-v1/;

/ {
        #address-cells = <0x2>;
        #size-cells = <0x2>;
        model = "Qualcomm Technologies, Inc. MSM 8998 v2.1 MTP";
        compatible = "qcom,msm8998-mtp", "qcom,msm8998", "qcom,mtp";
$
$
```

## SELINUX

> 目前仅支持下列语句

```bash
# 注意，这里面的 file class 只是 demo，你可以替换为别的 class
create xxxx                        # 新增一个 type
permissive xxxx                    # 设置 type 为 permissive
enforce xxxx                       # 设置 type 为 enforcing
allow xxxx bbbb:file *             # 允许 xxxx 对 bbbb 文件的所有操作
disallow xxxx bbbb:file *          # 禁止 xxxx 对 bbbb 文件的所有操作
allow xxxx cccc:file open          # 运行 xxxx 对 cccc 文件的 open 操作（注意你没给 write,read 这些这句其实没什么实际意义，只作为例子）
disallow xxxx cccc:file open       # 禁止 xxxx 对 cccc 文件的 open 操作
```

> 即时模式（运行时，重启失效）
```bash
# -l 意为 `load`，-s 意为 `语句`（注意使用单引号）
$ bxxt sepol -s 'create my_type' -l
```

> 永久模式（需要指定文件）
```bash
# 设置常量（sepolicy 路径)
$ export SEPOLICY=/sepolicy
# 永久设置 my_type 为 permissive
# -i 意为 `输入文件` -o 意为 `输出文件`
$ bxxt sepol -i $SEPOLICY -s 'permissive my_type' -o $SEPOLICY
```

OK，好了，知道了这些以后，如果你对这方面有所了解的话，应该就知道做什么了。

## 这是什么

编译好的全平台可执行文件在此下载 [https://github.com/rev1si0n/bxxt-binaries](https://github.com/rev1si0n/bxxt-binaries)
(我不太会使用自动 release)

bxxt 是作者为了掩盖自己能力不足不能够聪明的使用轮子而新建的一个轮子。这个轮子专门设计为运行在安卓设备上，
你可以在任何手机或者安卓模拟器上使用它。（注意，这个东西不能在宿主机上运行）

目前的功能:

* 可以完美的运行在 twrp 中，这意味着可以在刷机包中使用它。

* 解包，打包 boot.img (或许也能用在 recovery.img 上)

> 它会解包目前boot.img最可能需要修改的部分，包括，boot.img 中的 ramdisk，
> kernel 以及 dt 区内的设备树，以及一个解压好的 kernel，为你省去可能出现操作失误的麻烦。

* 编辑，注入 sepolicy

* 简单的 patch 二进制文件

* 设置安卓只读属性，无需重启，全局生效（APP系统调用、命令行），请看下列使用方法及注意事项

大部分工具可以在无 root 的情况下正常运行，但是实时selinux注入以及设置只读属性需要你使用root身份运行。

不幸的是，这个工具可能只支持官方的镜像格式，且目前只支持安卓 6.0 - 10，
由于部分安卓11设备仍使用 v2 镜像所以这个工具可能适用于部分安卓11的镜像。

关于我的编码风格表示抱歉，由于缩进等，它可能不太容易阅读。

## 如何编译

非常简单，你只需要确保你下载了 android-ndk-r20b

```bash
$ cd /path/to/bxxt;
$ /path/to/android-ndk-r20b/build/ndk-build -j 8;
...
$ ls -l libs/*
```

如果你的机器系统是 linux 或者 mac 并且运行着 docker，你也可以

```bash
$ cd /path/to/bxxt; bash build.sh;
```

## 如何使用

* 解包 boot.img

```bash
$ # 首先创建一个空目录用于存放解包后的文件
$ mkdir out
$ bxxt boot -i boot.img -o out/
```

* 解包的 boot.img 输出结构

```bash
$
$ # 这里可能会有 dt.dts-xx 文件（测试镜像里没有所以解包的演示也就没有）, 与 kernel.dts-xx 的意义基本相同，你也可以编辑
$ #
$ # 可能会产生一个文件叫做 extra.data，这是 bxxt 无法识别或者处理的数据
$ # 这个 extra.data 可能只是一个安卓证书，或者 AVB 或者非常规结构的 DTB，bxxt 会在后期将其追加到镜像。
$ # 提示：(如果这里面存的是 DTB或者AVB，你可能需要酌情编辑它来绕过某些AVB或者什么)
$ #
$ ls /path/to/out
-rw-rw-rw-    1 0        0              738 Mar 27 10:31 METADATA       # 元数据（修改命令行等）
-rw-------    1 0        0         39450632 Mar 27 10:31 kernel         # 已解压的 kernel
-rw-rw-rw-    1 0        0           624384 Mar 27 10:31 kernel.dts-00  # 设备树，文本，可编辑
-rw-rw-rw-    1 0        0           633469 Mar 27 10:31 kernel.dts-01
-rw-rw-rw-    1 0        0           624449 Mar 27 10:31 kernel.dts-02
drwxr-xr-x    9 0        0             4096 Mar 27 10:31 ramdisk        # randisk
-rw-r--r--    1 0        0                0 Mar 26 18:02 recovery_dtbo  # recovery_dtbo
-rw-r--r--    1 0        0                0 Mar 26 18:02 second         # second
$
$ ls /path/to/out/ramdisk
drwxr-xr-x    2 0        0             4096 Jan  1  1970 apex
drwxr-xr-x    2 0        0             4096 Jan  1  1970 debug_ramdisk
drwxr-xr-x    2 0        0             4096 Jan  1  1970 dev
-rwxr-x---    1 0        0          1891328 Jan  1  1970 init
drwxr-xr-x    2 0        0             4096 Jan  1  1970 mnt
drwxr-xr-x    2 0        0             4096 Jan  1  1970 proc
drwxr-xr-x    2 0        0             4096 Jan  1  1970 sys
-rw-r--r--    1 0        0              524 Jan  1  1970 verity_key
$
$ # 你可以编辑 METADATA 中的任意行除了下面标记为 `不要编辑` 的这些
$ cat /path/to/out/METADATA
bxxt.kernel_addr=8000
bxxt.ramdisk_addr=1000000
bxxt.second_addr=f00000
bxxt.tags_addr=100
bxxt.dtb_addr=0
bxxt.recovery_dtbo_offset=0
bxxt.name=
bxxt.cmdline=androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x37
bxxt.extra_cmdline=
bxxt.kernel_compression=1             # ! 不要编辑
bxxt.header_version=0                 # ! 不要编辑
bxxt.os_version=14000144
bxxt.header_size=0                    # ! 不要编辑
bxxt.page_size=1000
bxxt.kernel_size=14890c7              # ! 不要编辑
bxxt.ramdisk_size=c5302               # ! 不要编辑
bxxt.second_size=0                    # ! 不要编辑
bxxt.recovery_dtbo_size=0             # ! 不要编辑
bxxt.dtb_size=0                       # ! 不要编辑
$
$ # 看一下解包好的 dts 文件，你可以酌情编辑
$ head /path/to/out/kernel.dts-00
/dts-v1/;

/ {
        #address-cells = <0x2>;
        #size-cells = <0x2>;
        model = "Qualcomm Technologies, Inc. MSM 8998 v2.1 MTP";
        compatible = "qcom,msm8998-mtp", "qcom,msm8998", "qcom,mtp";
$
```

* 重新打包 boot.img
> 注意，某些情况下生成的 boot_modified.img 大小可能超过你实际 boot 分区的大小
> 如果在你刷入时 boot_modified.img 出现 size too large 这种错误,
> 尝试增加一个命令行选项 -e skip-unknown-data
> 如 `bxxt -i out/ -o boot_modified.img -e skip-unknown-data`
> 重新生成镜像并刷入

```bash
$ bxxt boot -i out/ -o boot_modified.img
```

* patch 二进制文件

```bash
$ # @ 代表 AT, @偏移:大小=字节序列, 这个 `大小` 最大是 8，也就是说一次只能改 8 个字节
$ # 这个字节序列代表你在任何十六进制编辑器中`看到的顺序`
$ # 如果十六进制编辑器显示为 00000000: 0A 0B 0C 0D 11 22 33 44
$ # 你想把 0A 0B 改为 CC EE，只需要
$ bxxt patch @00000000:2=ccee /path/to/binary/file
$ # 或者你想把这一整个 `0A 0B 0C 0D 11 22 33 44` 改为 `01 02 03 04 05 06 07 08`
$ # 只需要
$ bxxt patch @00000000:8=0102030405060708 /path/to/binary/file
$
$ # 禁用 vbmeta 校验的例子
$ bxxt patch @00000078:4=00000002 /dev/block/by-name/vbmeta_a
```

* sepolicy 编辑，注入

注意使用 `单引号` 把 `-s` 参数包裹起来。不建议直接 setenforce，特事特办，需要什么权限给什么权限。

> 编辑 sepolicy 文件

```bash
$ # 输入 (-i) 以及输出 (-o) 文件可以是相同的（如果你不想创建一个中间文件的话）
$ bxxt sepol -s 'create deltaforce' -i /path/to/sepolicy -o /path/to/out/sepolicy
```

> 即时模式 (在运行的安卓系统中即时生效)

```bash
$ # 即时模式样例
$ bxxt sepol -s 'create deltaforce' -l
```

> 所有支持的语句 (-s)

```bash
create aaaa # 新增一个域
permissive aaaa # permissive 一个域
enforce aaaa # enforce 一个域
allow aaaa bbbb:file * # 允许域 aaaa 对 域 bbbb 的文件的所有操作
disallow aaaa bbbb:file * # 反之
allow aaaa bbbb:file open # 仅允许域 aaaa 对 域 bbbb 的文件的 open 操作
disallow aaaa bbbb:file open # 反之
```

* 设置只读属性 （无需重启，全局生效）

```bash
$ bxxt setprop ro.build.fingerprint whathell
```

特别注意：设置 `ro.debuggable` 为 `1` 并不会得到你想要的结果，因为安卓已经在启动时读取并赋值了这个变量，
所以这里的更改相对来说是无效的！如果你想养你的设备变为可调试状态，你应该使用

```bash
$ bxxt setdebuggable 1
$ # bxxt setdebuggable 0
```

但是注意！这会使你的设备进行热重启，你的设备会表现出重启的状态！

## 版权

查看根目录的 `COPYING` 文件

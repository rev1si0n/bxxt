## WHAT IT IS

[简体中文介绍](README_zh.md)

prebuilt all platform binaries download from here [https://github.com/rev1si0n/bxxt-binaries](https://github.com/rev1si0n/bxxt-binaries)
(idk how to use github auto release)

bxxt is another wheel designed for noob author's needs.
now it can only run in the android host, it means you can only
run this tool in the phone or emulator (not your computer)

feature:
* run perfectly in twrp environment

* pack/unpack boot.img (may be also recovery.img)

> include any device tree in the boot.img, it will decompile it and show you the plain text.
> and also decompressed kernel.

* edit/inject selinux/sepolicy

* patch binary file

* set android readonly property (ro.x) , global effect (apps/commands), see usage below for details


most tools can run without root, but run-time sepolicy inject and set
readonly property needs you run as root.

the bad news is it only support android 6.0 - 10.0,
may be support android 11 (depends on the boot.img version).

sorry about my code style, it may hard to read.

## HOW TO BUILD

easy and simple, but you need to download android-ndk-r20b first.

```bash
$ cd /path/to/bxxt;
$ /path/to/android-ndk-r20b/build/ndk-build -j 8;
...
$ ls -l libs/*
```

if you are using linux/macos with docker and don't want to install ndk in the host, you can also

```bash
$ cd /path/to/bxxt; bash build.sh;
```

## BASIC USAGE

* unpack boot.img

```bash
$ # create an empty output directory first
$ mkdir out
$ bxxt boot -i boot.img -o out/
```

* unpacked boot.img structure

```bash
$
$ # there's may be some dt.dts-xx files, means as same as kernel.dts-xx
$ #
$ # there also may exist a file named extra.data, it's the data we cant
$ # recognize or processed. this extra.data usually an android certificate
$ # or avb or unknown dtb. we need to append it when you pack the boot.img.
$ #
$ ls /path/to/out
-rw-rw-rw-    1 0        0              738 Mar 27 10:31 METADATA       # meta data
-rw-------    1 0        0         39450632 Mar 27 10:31 kernel         # decompressed linux kernel
-rw-rw-rw-    1 0        0           624384 Mar 27 10:31 kernel.dts-00  # device tree (plain text, you can modify it)
-rw-rw-rw-    1 0        0           633469 Mar 27 10:31 kernel.dts-01
-rw-rw-rw-    1 0        0           624449 Mar 27 10:31 kernel.dts-02
drwxr-xr-x    9 0        0             4096 Mar 27 10:31 ramdisk        # ramdisk directory
-rw-r--r--    1 0        0                0 Mar 26 18:02 recovery_dtbo  # recovery_dtbo file
-rw-r--r--    1 0        0                0 Mar 26 18:02 second         # second file
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
$ # you can modify any lines in METADATA except marked as `DO NOT MODIFY` below
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
bxxt.kernel_compression=1             # ! DO NOT MODIFY
bxxt.header_version=0                 # ! DO NOT MODIFY
bxxt.os_version=14000144
bxxt.header_size=0                    # ! DO NOT MODIFY
bxxt.page_size=1000
bxxt.kernel_size=14890c7              # ! DO NOT MODIFY
bxxt.ramdisk_size=c5302               # ! DO NOT MODIFY
bxxt.second_size=0                    # ! DO NOT MODIFY
bxxt.recovery_dtbo_size=0             # ! DO NOT MODIFY
bxxt.dtb_size=0                       # ! DO NOT MODIFY
$
$ # decompiled dts file (if exist), you can also modify it
$ head /path/to/out/kernel.dts-00
/dts-v1/;

/ {
        #address-cells = <0x2>;
        #size-cells = <0x2>;
        model = "Qualcomm Technologies, Inc. MSM 8998 v2.1 MTP";
        compatible = "qcom,msm8998-mtp", "qcom,msm8998", "qcom,mtp";
$
```

* pack boot.img

> NOTICE: this may generate a image that size exceeds your boot partition is.
> if size too large error occurs when you flash the boot_modified.img,
> try add extra option -e skip-unknown-data like `bxxt -i out/ -o boot_modified.img -e skip-unknown-data`
> regenrate the image and flash it

```bash
$ bxxt -i out/ -o boot_modified.img
```

* patch binary

```bash
$ # @ means AT, @offset:size=to_byte_seq, the max `size` is 8
$ # that to_byte_seq is what you see in any hex editor
$ # if hex editor shows 00000000: 0A 0B 0C 0D 11 22 33 44
$ # and you want to modify 0A 0B to CC EE, just use
$ bxxt patch @00000000:2=ccee /path/to/binary/file
$ # or if you want the whole `0A 0B 0C 0D 11 22 33 44` to `01 02 03 04 05 06 07 08`
$ # use
$ bxxt patch @00000000:8=0102030405060708 /path/to/binary/file
$
$ # disable vbmeta verification example
$ bxxt patch @00000078:4=00000002 /dev/block/by-name/vbmeta_a
```

* sepolicy modify, inject

remember to use `single quote` to wrap around the `-s` parameter.

> modify sepolicy file

```bash
$ # input (-i) and output (-o) file can be the same
$ bxxt sepol -s 'create deltaforce' -i /path/to/sepolicy -o /path/to/out/sepolicy
```

> live mode (running system)

```bash
$ # live mode example
$ bxxt sepol -s 'create deltaforce' -l
```

> all supported commands (-s)

```bash
create aaaa # create a domain
permissive aaaa # permissive a domain
enforce aaaa # enforce a domain
allow aaaa bbbb:file * # allow doamin aaaa's all operations to bbbb's file
disallow aaaa bbbb:file *
allow aaaa bbbb:file open # only allow doamin aaaa's open operation to bbbb's file
disallow aaaa bbbb:file open
```

* set readonly property (without reboot)

```bash
$ bxxt setprop ro.build.fingerprint whathell
```

notice: set `ro.debuggable` to `1` will not get the result you want
cause android already initialized this attribute
to a const variable at the boot time, if you want all apps are debuggable,
you should use:

```bash
$ bxxt setdebuggable 1
$ # bxxt setdebuggable 0
```

this will let your device a `HOT-REBOOT`, your device will behave like a normal reboot but
only the android service over the linux kernel.

## LICENSE

see `COPYING` under the source root.

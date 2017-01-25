#openwrt/LEDE source dir
WRTP = /home/prog/openwrt/lede-all/new-lede-rb941/source
#this var is need for compile. It points to toolchain dir.
export STAGING_DIR = $(WRTP)/staging_dir/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.16
#where is a compiler binarys(gcc, ldd)
PATH := $(PATH):$(STAGING_DIR)/bin
#arch
export ARCH = mips
#cross-compile-prefix
export CROSS_COMPILE = mips-openwrt-linux-
#path to ready kernel sources(inside LEDE)
KERNEL_DIR = $(WRTP)/build_dir/target-mips_24kc_musl-1.1.16/linux-ar71xx_mikrotik/linux-4.4.42

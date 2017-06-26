export KBUILD_BUILD_USER="DarkBlood"
export KBUILD_BUILD_HOST=""

export ARCH=arm64

make clean && make mrproper

make k5fpr_defconfig

make -j2


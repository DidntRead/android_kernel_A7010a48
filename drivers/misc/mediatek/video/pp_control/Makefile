MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/include/ \
	     -I$(srctree)/drivers/misc/mediatek/video/ \
 	     -I$(srctree)/drivers/misc/mediatek/lcm/inc/ \
	     -I$(srctree)/drivers/misc/mediatek/cmdq/v2/ \
             -I$(srctree)/drivers/misc/mediatek/cmdq/v2/$(MTK_PLATFORM)/ \
	     -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys

ifeq ($(CONFIG_ARCH_MT6735),y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/mt6735/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/mt6735/
endif
ifeq ($(CONFIG_ARCH_MT6735M),y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/mt6735m/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/mt6735m/
endif
ifeq ($(CONFIG_ARCH_MT6753),y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/mt6753/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/mt6753/
endif

subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmp
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/mt6735

obj-y := pp_control.o




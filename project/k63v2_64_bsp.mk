#
LOCAL_DIR := $(GET_LOCAL_DIR)
TARGET := k63v2_64_bsp
MODULES += app/mt_boot \
           dev/lcm
MTK_EMMC_SUPPORT=yes
MTK_KERNEL_POWER_OFF_CHARGING = yes
MTK_SMI_SUPPORT = yes
DEFINES += MTK_GPT_SCHEME_SUPPORT
MTK_PUMP_EXPRESS_PLUS_SUPPORT := yes
MTK_MT6370_PMU_CHARGER_SUPPORT := yes
MTK_MT6370_PMU_BLED_SUPPORT := yes
MTK_CHARGER_INTERFACE := yes
MTK_LCM_PHYSICAL_ROTATION = 0
CUSTOM_LK_LCM="hx83112_fhdp_dsi_vdo_auo"
#nt35595_fhd_dsi_cmd_truly_nt50358 = yes
MTK_SECURITY_SW_SUPPORT = yes
MTK_VERIFIED_BOOT_SUPPORT = yes
MTK_SEC_FASTBOOT_UNLOCK_SUPPORT = yes
SPM_FW_USE_PARTITION = yes
BOOT_LOGO := fhdplus2340
DEBUG := 2
#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
CUSTOM_LK_USB_UNIQUE_SERIAL=no
MTK_TINYSYS_SCP_SUPPORT = no
MTK_TINYSYS_SSPM_SUPPORT = yes
#DEFINES += NO_POWER_OFF=y
DEFINES += MTK_NEW_COMBO_EMMC_SUPPORT
#DEFINES += FPGA_BOOT_FROM_LK=y
MTK_PROTOCOL1_RAT_CONFIG = Lf/W/G
MTK_GOOGLE_TRUSTY_SUPPORT = no
DEFINES += MTK_MT6370_PMU
MTK_DM_VERITY_OFF = no
MTK_DYNAMIC_CCB_BUFFER_GEAR_ID =
MTK_AVB20_SUPPORT:=yes
MTK_CW2015_SUPPORT = yes
#prize wangwei AndroidP OTA to AndroidQ 20191106 begin
SYSTEM_AS_ROOT = yes
#prize wangwei AndroidP OTA to AndroidQ 20191106 end

#prize-add by sunshuai for 5715 wireless charge start
PRIZE_WIRELESS_RECEIVER_MAXIC_MT5715 = yes
export PRIZE_WIRELESS_RECEIVER_MAXIC_MT5715
#prize-add by sunshuai for 5715 wireless charge end
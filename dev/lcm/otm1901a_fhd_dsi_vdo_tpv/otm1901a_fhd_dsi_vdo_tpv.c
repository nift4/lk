/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#define LOG_TAG "LCM"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#include <debug.h>
#ifndef MACH_FPGA
#include <lcm_pmic.h>
#endif
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_pm_ldo.h>
#include <mach/mt_gpio.h>
#endif

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(CRITICAL, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(INFO, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_notice("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

#define LCM_ID_NT35695 (0xf5)

static const unsigned int BL_MIN_LEVEL = 20;
static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define MDELAY(n)       (lcm_util.mdelay(n))
#define UDELAY(n)       (lcm_util.udelay(n))

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
        lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
      lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
        lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
/* #include <linux/jiffies.h> */
/* #include <linux/delay.h> */
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif

/* static unsigned char lcd_id_pins_value = 0xFF; */
static const unsigned char LCD_MODULE_ID = 0x01;
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define LCM_DSI_CMD_MODE                                    0
#define FRAME_WIDTH                                     (1080)
#define FRAME_HEIGHT                                    (1920)
#define GPIO_OUT_ONE  1
#define GPIO_OUT_ZERO 0

/* GPIO158       panel 1.8V for controll */
#ifdef GPIO_LCM_PWR_EN
#define GPIO_LCD_PWR_EN      GPIO_LCM_PWR_EN
#else
#define GPIO_LCD_PWR_EN		0xffffffff
#endif

/* GPIO159       panel 1.8V for controll */
#ifdef GPIO_LCM_PWR2_EN
#define GPIO_LCD_PWR2_EN      GPIO_LCM_PWR2_EN
#else
#define GPIO_LCD_PWR2_EN		0xffffffff
#endif

#define REGFLAG_DELAY       	0xFFFC
#define REGFLAG_UDELAY  		0xFFFB
#define REGFLAG_END_OF_TABLE    0xFFFD
#define REGFLAG_RESET_LOW   	0xFFFE
#define REGFLAG_RESET_HIGH  	0xFFFF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} },
	{0x4F, 1, {0x01} },
	{REGFLAG_DELAY, 120, {} }
};

static struct LCM_setting_table init_setting[] = {
	{0x00, 1, {0x00}},
	{0xFF, 4, {0x19,0x01,0x01,0x00}},
	{0x00, 1, {0x80}},
	{0xFF, 2, {0x19,0x01}},
	{0x00, 1, {0x00}},
	{0x1C, 1, {0x33}},
	{0x00, 1, {0xA0}},
	{0xC1, 1, {0xE8}},
	{0x00, 1, {0xA7}},
	{0xC1, 1, {0x00}},
	{0x00, 1, {0x90}},
	{0xC0, 6, {0x00,0x2F,0x00,0x00,0x00,0x01}},
	{0x00, 1, {0xC0}},
	{0xC0, 6, {0x00,0x2F,0x00,0x00,0x00,0x01}},
	{0x00, 1, {0x9A}},
	{0xC0, 1, {0x1E}},
	{0x00, 1, {0xAC}},
	{0xC0, 1, {0x06}},
	{0x00, 1, {0xDC}},
	{0xC0, 1, {0x06}},
	{0x00, 1, {0x81}},
	{0xA5, 1, {0x06}},
	{0x00, 1, {0x82}},
	{0xC4, 1, {0xF0}},
	{0x00, 1, {0x92}},
	{0xE9, 1, {0x00}},
	{0x00, 1, {0x90}},
	{0xF3, 1, {0x01}},
	{0x00, 1, {0x82}},
	{0xA5, 1, {0x1F}},
	{0x00, 1, {0x93}},
	{0xC5, 1, {0x19}},
	{0x00, 1, {0x95}},
	{0xC5, 1, {0x28}},
	{0x00, 1, {0x97}},
	{0xC5, 1, {0x18}},
	{0x00, 1, {0x99}},
	{0xC5, 1, {0x23}},
	{0x00, 1, {0x9B}},
	{0xC5, 2, {0x44,0x40}},
	{0x00, 1, {0x00}},
	{0xD9, 2, {0x00,0xBA}},
	{0x00, 1, {0x00}},
	{0xD8, 2, {0x1B,0x1B}},
	{0x00, 1, {0xB3}},
	{0xC0, 1, {0xCC}},
	{0x00, 1, {0xBC}},
	{0xC0, 1, {0x00}},
	{0x00, 1, {0x84}},
	{0xC4, 1, {0x22}},
	{0x00, 1, {0x94}},
	{0xC1, 1, {0x84}},
	{0x00, 1, {0x98}},
	{0xC1, 1, {0x74}},
	{0x00, 1, {0x80}},
	{0xC4, 1, {0x38}},
	{0x00, 1, {0xCD}},
	{0xF5, 1, {0x19}},
	{0x00, 1, {0xDB}},
	{0xF5, 1, {0x19}},
	{0x00, 1, {0xF5}},
	{0xC1, 1, {0x40}},
	{0x00, 1, {0xB9}},
	{0xC0, 1, {0x11}},
	{0x00, 1, {0x8D}},
	{0xF5, 1, {0x20}},
	{0x00, 1, {0x80}},
	{0xC0, 14, {0x00,0x86,0x00,0x0A,0x0A,0x00,0x86,0x0A,0x0A,0x00,0x86,0x00,0x0A,0x0A}},
	{0x00, 1, {0xF0}},
	{0xC3, 6, {0x00,0x00,0x00,0x00,0x00,0x80}},
	{0x00, 1, {0xA0}},
	{0xC0, 7, {0x00,0x00,0x03,0x00,0x00,0x1E,0x06}},
	{0x00, 1, {0xD0}},
	{0xC0, 7, {0x00,0x00,0x03,0x00,0x00,0x1E,0x06}},
	{0x00, 1, {0x90}},
	{0xC2, 4, {0x84,0x01,0x3B,0x40}},
	{0x00, 1, {0xB0}},
	{0xC2, 8, {0x02,0x01,0x45,0x43,0x02,0x01,0x45,0x43}},
	{0x00, 1, {0x80}},
	{0xC3, 12, {0x84,0x08,0x03,0x00,0x02,0x89,0x82,0x08,0x03,0x00,0x02,0x89}},
	{0x00, 1, {0x90}},
	{0xC3, 12, {0x83,0x08,0x03,0x00,0x02,0x89,0x81,0x08,0x03,0x00,0x02,0x89}},
	{0x00, 1, {0x80}},
	{0xCC, 15, {0x09,0x0D,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x0E,0x28,0x28,0x28,0x28}},
	{0x00, 1, {0x90}},
	{0xCC, 15, {0x0D,0x09,0x14,0x13,0x12,0x11,0x15,0x16,0x17,0x18,0x0E,0x28,0x28,0x28,0x28}},
	{0x00, 1, {0xA0}},
	{0xCC, 15, {0x1D,0x1E,0x1F,0x19,0x1A,0x1B,0x1C,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27}},
	{0x00, 1, {0xB0}},
	{0xCC, 8, {0x01,0x02,0x03,0x05,0x06,0x07,0x04,0x08}},
	{0x00, 1, {0xC0}},
	{0xCC, 12, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x77}},
	{0x00, 1, {0xD0}},
	{0xCC, 12, {0xFF,0x0F,0x30,0xC0,0x0F,0x30,0x00,0x00,0x33,0x03,0x00,0x77}},
	{0x00, 1, {0xDE}},
	{0xCC, 1, {0x00}},
	{0x00, 1, {0x80}},
	{0xCB, 15, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00}},
	{0x00, 1, {0x90}},
	{0xCB, 15, {0x30,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00, 1, {0xA0}},
	{0xCB, 15, {0x15,0x15,0x05,0xF5,0x05,0xF5,0x00,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x00}},
	{0x00, 1, {0xB0}},
	{0xCB, 15, {0x00,0x01,0xFD,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00, 1, {0xC0}},
	{0xCB, 8, {0x00,0x00,0x00,0x00,0x00,0x00,0x77,0x77}},
	{0x00, 1, {0xD0}},
	{0xCB, 8, {0x00,0x00,0x00,0x00,0x00,0x00,0x77,0x77}},
	{0x00, 1, {0xE0}},
	{0xCB, 8, {0x00,0x00,0x00,0x01,0x01,0x01,0x77,0x77}},
	{0x00, 1, {0xF0}},
	{0xCB, 8, {0x11,0x11,0x11,0x11,0x11,0x11,0x77,0x77}},
	{0x00, 1, {0x80}},
	{0xCD, 15, {0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x01,0x12,0x11,0x03,0x04,0x0B,0x17}},
	{0x00, 1, {0x90}},
	{0xCD, 11, {0x3D,0x02,0x3D,0x25,0x25,0x25,0x1F,0x20,0x21,0x25,0x25}},
	{0x00, 1, {0xA0}},
	{0xCD, 15, {0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x01,0x12,0x11,0x05,0x06,0x0B,0x17}},
	{0x00, 1, {0xB0}},
	{0xCD, 11, {0x17,0x02,0x3D,0x25,0x25,0x25,0x1F,0x20,0x21,0x25,0x25}},
	{0x00, 1, {0x00}},
	{0xE1, 24, {0x56,0x56,0x59,0x60,0x64,0x67,0x6d,0x79,0x7e,0x8c,0x93,0x99,0x62,0x5e, 0x5e,0x4f, 0x3e,0x2f,0x24,0x1d,0x16,0x0c,0x08,0x04}},
	{0x00, 1, {0x00}},
	{0xE2, 24, {0x56,0x56,0x59,0x60,0x64,0x67,0x6d,0x79,0x7e,0x8c,0x93,0x99,0x62,0x5e, 0x5a,0x4b, 0x3e,0x2f,0x24,0x1d,0x16,0x0c,0x08,0x04}},
	{0x00, 1, {0x00}},
	{0xE3, 24, {0x53,0x56,0x58,0x5c,0x61,0x65,0x6c,0x77,0x7c,0x8b,0x93,0x99,0x62,0x5e, 0x5d,0x4f, 0x3e,0x2e,0x24,0x1d,0x16,0x0c,0x07,0x04}},
	{0x00, 1, {0x00}},
	{0xE4, 24, {0x53,0x56,0x58,0x5c,0x61,0x65,0x6c,0x77,0x7c,0x8b,0x93,0x99,0x62,0x5e, 0x59,0x4b, 0x3e,0x2e,0x24,0x1d,0x16,0x0c,0x07,0x04}},
	{0x00, 1, {0x00}},
	{0xE5, 24, {0x20,0x22,0x29,0x35,0x3f,0x45,0x51,0x63,0x6e,0x81,0x8c,0x95,0x64,0x5f, 0x5e,0x4e, 0x3e,0x2e,0x24,0x1d,0x16,0x0c,0x07,0x04}},
	{0x00, 1, {0x00}},
	{0xE6, 24, {0x20,0x22,0x29,0x35,0x3f,0x45,0x51,0x63,0x6e,0x81,0x8c,0x95,0x64,0x5f, 0x5a,0x4a, 0x3e,0x2e,0x24,0x1d,0x16,0x0c,0x07,0x04}},
	{0x00, 1, {0xD4}},
	{0xC3, 4, {0x01,0x01,0x01,0x01}},
	{0x00, 1, {0xF7}},
	{0xC3, 4, {0x03,0x1B,0x00,0x00}},
	{0x00, 1, {0xF2}},
	{0xC1, 3, {0x80,0x0F,0x0F}},
	{0x00, 1, {0xC2}},
	{0xC5, 1, {0x12}},
	{0x00, 1, {0xA8}},
	{0xC4, 1, {0x11}},
	{0x00, 1, {0x00}},
	{0xFF, 3, {0xFF,0xFF,0xFF}},

	{0x11, 0, {} },
	{REGFLAG_DELAY, 120, {} },

	{0x29, 0, {} },
	{REGFLAG_DELAY, 50, {} },
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {

			case REGFLAG_DELAY:
				if (table[i].count <= 10)
					MDELAY(table[i].count);
				else
					MDELAY(table[i].count);
				break;

			case REGFLAG_UDELAY:
				UDELAY(table[i].count);
				break;

			case REGFLAG_END_OF_TABLE:
				break;

			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
#endif
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch = 8;
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_frontporch_for_low_power = 620;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 10;
	params->dsi.horizontal_backporch = 20;
	params->dsi.horizontal_frontporch = 40;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	/* params->dsi.ssc_disable                                                   = 1; */
#ifndef MACH_FPGA
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 420;	/* this value must be in MTK suggested table */
#else
	params->dsi.PLL_CLOCK = 440;	/* this value must be in MTK suggested table */
#endif
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	params->dsi.CLK_HS_POST = 36;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd = 0x53;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x24;
}

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */
static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
#ifdef BUILD_LK
	mt_set_gpio_mode(GPIO, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO, output);
#else
	gpio_set_value(GPIO, output);
#endif
}

static void lcm_init_power(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_init_power() enter\n");

	lcm_set_gpio_output(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ONE);
	MDELAY(20);

	SET_RESET_PIN(0);
	MDELAY(5);

	SET_RESET_PIN(1);
	MDELAY(20);
#else
	pr_err("[Kernel/LCM] lcm_init_power() enter\n");
#endif
}

static void lcm_suspend_power(void)
{
	SET_RESET_PIN(0);
	MDELAY(10);

	lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ZERO);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_PWR_EN, GPIO_OUT_ZERO);
	MDELAY(20);
}

static void lcm_resume_power(void)
{
	lcm_set_gpio_output(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);
	MDELAY(20);

	lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ONE);
	MDELAY(20);

	SET_RESET_PIN(1);
	MDELAY(20);
}

static void lcm_init(void)
{
	push_table(init_setting, sizeof(init_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
	lcm_init();
}

static void lcm_update(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#if 0
static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0, version_id = 0;
	unsigned char buffer[2];
	unsigned int array[16];

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);

	SET_RESET_PIN(1);
	MDELAY(20);

	array[0] = 0x00023700;	/* read id return two byte,version and id */
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0];     /* we only need ID */

	read_reg_v2(0xDB, buffer, 1);
	version_id = buffer[0];

	LCM_LOGI("%s,nt35695_id=0x%08x,version_id=0x%x\n", __func__, id, version_id);

	if (id == LCM_ID_NT35695 && version_id == 0x81)
		return 1;
	else
		return 0;
}

/* return TRUE: need recovery */
/* return FALSE: No need recovery */
static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
	char buffer[3];
	int array[4];

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x53, buffer, 1);

	if (buffer[0] != 0x24) {
		LCM_LOGI("[LCM ERROR] [0x53]=0x%02x\n", buffer[0]);
		return TRUE;
	}
	LCM_LOGI("[LCM NORMAL] [0x53]=0x%02x\n", buffer[0]);
	return FALSE;
#else
	return FALSE;
#endif
}
#endif
static unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
	unsigned int ret = 0;
	unsigned int x0 = FRAME_WIDTH / 4;
	unsigned int x1 = FRAME_WIDTH * 3 / 4;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);

	unsigned int data_array[3];
	unsigned char read_buf[4];
	LCM_LOGI("ATA check size = 0x%x,0x%x,0x%x,0x%x\n", x0_MSB, x0_LSB, x1_MSB, x1_LSB);
	data_array[0] = 0x0005390A; /* HS packet */
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00043700; /* read id return two byte,version and id */
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0x2A, read_buf, 4);

	if ((read_buf[0] == x0_MSB) && (read_buf[1] == x0_LSB)
	        && (read_buf[2] == x1_MSB) && (read_buf[3] == x1_LSB))
		ret = 1;
	else
		ret = 0;

	x0 = 0;
	x1 = FRAME_WIDTH - 1;

	x0_MSB = ((x0 >> 8) & 0xFF);
	x0_LSB = (x0 & 0xFF);
	x1_MSB = ((x1 >> 8) & 0xFF);
	x1_LSB = (x1 & 0xFF);

	data_array[0] = 0x0005390A; /* HS packet */
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	return ret;
#else
	return 0;
#endif
}

LCM_DRIVER otm1901a_fhd_dsi_vdo_tpv_lcm_drv = {
	.name = "otm1901a_fhd_dsi_vdo_tpv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
/*	.compare_id = lcm_compare_id, */
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
/*	.esd_check = lcm_esd_check, */
	.ata_check = lcm_ata_check,
	.update = lcm_update,
};

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
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_pm_ldo.h>
#include <mach/mt_gpio.h>
#endif

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)			(lcm_util.set_reset_pin((v)))
#define MDELAY(n)					(lcm_util.mdelay(n))

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
	lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) \
	lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
	lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
	lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
#define set_gpio_lcd_enp(cmd) \
		lcm_util.set_gpio_lcd_enp_bias(cmd)
		

static const unsigned char LCD_MODULE_ID = 0x01;
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define LCM_DSI_CMD_MODE	0
#define FRAME_WIDTH  										 (720)
#define FRAME_HEIGHT 										 (1600)

#define REGFLAG_DELAY             							 0xFFFA
#define REGFLAG_UDELAY             							 0xFFFB
#define REGFLAG_PORT_SWAP									 0xFFFC
#define REGFLAG_END_OF_TABLE      							 0xFFFD   // END OF REGISTERS MARKER

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

struct LCM_setting_table
{
    unsigned int cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] =
{
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 60, {} },
};

static struct LCM_setting_table lcm_initialization_setting[] =
{

 	#if 0

        {0x03,1,{0xB9}},
        {0x83,2,{0x10,0x2D}},
        {0x0B,1,{0xB1}},
        {0x22,9,{0x44,0x27,0x27,0x32,0x52,0x57,0x39,0x08,0x08}},
        {0x08,1,{0x00}},
        {0x0E,1,{0xB2}},
        {0x00,9,{0x58,0x01,0x58,0x01,0x58,0x03,0x58,0x03,0xFF}},
        {0x01,3,{0x20,0x00,0xFF,}},
        {0x0E,1,{0xB4}},
        {0x01,9,{0x58,0x01,0x58,0x01,0x58,0x03,0x58,0x03,0xFF}},
        {0x01,3,{0x01,0x20,0x00,0xFF}},
        {0x01,1,{0xCC}},
        {0x02,1,{0x00}},
        {0x19,1,{0xD3}},
        {0x00,9,{0x25,0x18,0x18,0x19,0x19,0x18,0x18,0x18,0x18}},
		{0x37,9,{0x0E,0x0E,0x00,0x00,0x32,0x10,0x08,0x00,0x08}},
		{0x32,9,{0x16,0x4E,0x06,0x4E,}},
        {0x2C,1,{0xD5}},
        {0x24,9,{0x25,0x18,0x18,0x19,0x19,0x18,0x18,0x18,0x18}},
        {0x18,9,{0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x06,0x07}},
        {0x04,9,{0x05,0x18,0x18,0x18,0x18,0x02,0x03,0x00,0x01}},
        {0x20,9,{0x21,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}},
        {0x18,3,{0x18,0x18,0x18}},
        {0x2C,1,{0xD6}},
        {0x21,9,{0x20,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}},
        {0x18,9,{0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x01,0x00}},
        {0x03,9,{0x02,0x18,0x18,0x18,0x18,0x05,0x04,0x07,0x06}},
        {0x25,9,{0x24,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}},
        {0x18,3,{0x18,0x18,0x18}},
        {0x04,1,{0xE7}},
        {0xFF,3,{0x14,0x00,0x00}},
        {0x01,1,{0xBD}},
        {0x01,1,{0x00}},
        {0x01,1,{0xE7}},
        {0x01,1,{0x00}},
        {0x01,1,{0xBD}},
        {0x01,1,{0x00}},
        {0x01,1,{0xBD}},
        {0x02,1,{0x00}},
        {0x0C,1,{0xD8}},
        {0xFF,9,{0xFF,0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0xFF}},
        {0xFF,1,{0xF0}},
        {0x01,1,{0xBD}},
        {0x03,1,{0x00}},
        {0x18,1,{0xD8}},
        {0xAA,9,{0xAA,0xAA,0xAA,0xAA,0xA0,0xAA,0xAA,0xAA,0xAA}},
        {0xAA,9,{0xA0,0xAA,0xAA,0xAA,0xAA,0xAA,0xA0,0xAA,0xAA}},
        {0xAA,1,{0xAA,0xAA,0xA0}},
        {0x01,1,{0xBD}},
        {0x00,1,{0x00}},
        {0x13,1,{0xBA}},
        {0x70,9,{0x23,0xA8,0x93,0xB2,0xC0,0xC0,0x01,0x10,0x00}},
        {0x00,9,{0x00,0x0D,0x3D,0x82,0x77,0x04,0x01,0x04}},
        {0x01,1,{0xBD}},
        {0x01,1,{0x00}},
        {0x01,1,{0xCB}},
        {0x01,1,{0x00}},
		{0x01,1,{0xBD}},
		{0x01,1,{0x00}},
        {0x05,1,{0xCB}},
        {0x00,4,{0x53,0x00,0x02,0x59}},
        {0x07,1,{0xBF}},
        {0xFC,6,{0x00,0x04,0x9E,0xF6,0x00,0x5D}},
        {0x01,1,{0xBD}},
		{0x02,1,{0x00}},
        {0x08,1,{0xB4}},
        {0x42,7,{0x00,0x33,0x00,0x33,0x88,0xB3,0x00}},
        {0x01,1,{0xBD}},
		{0x00,1,{0x00}},
        {0x02,1,{0xD1}},
        {0x20,1,{0x01}},
		{0x01,1,{0xBD}},
		{0x02,1,{0x00}},
        {0x03,1,{0xB1}},
        {0x7F,2,{0x03,0xF5}},
		{0x01,1,{0xBD}},
		{0x00,1,{0x00}},
        {0x90,1,{0xA5}},
        {0x41,2,{0x00,0x2F}},
#endif
		{0x11,0,{}},
			{REGFLAG_DELAY,60, {}},
			{0x29,0,{}},
			{REGFLAG_DELAY,50, {}},
			//prize-add-pengzhipeng-20200313-start
			{0xB9,3,{0x83,0x10,0x2D}},
			{REGFLAG_DELAY,10, {}},
			{0xCD,1,{0x01}},
			{REGFLAG_DELAY,10, {}},
			{0xB9,3,{0x00,0x00,0x00}},
			//prize-add-pengzhipeng-20200313-end
			{REGFLAG_DELAY,10, {}}, 
		{REGFLAG_END_OF_TABLE, 0x00, {}}    
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for (i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd)
        {
        case REGFLAG_DELAY:
            if (table[i].count <= 10)
                MDELAY(table[i].count);
            else
                MDELAY(table[i].count);
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

	params->type                         = LCM_TYPE_DSI;
	params->width                        = FRAME_WIDTH;
	params->height                       = FRAME_HEIGHT;

	#ifndef BUILD_LK
	params->physical_width               = 68;
	params->physical_height              = 151;
	params->physical_width_um            = 67930;
	params->physical_height_um           = 150960;
	params->density                      = 320;
	#endif

	// enable tearing-free
	params->dbi.te_mode                  = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity         = LCM_POLARITY_RISING;

	#if (LCM_DSI_CMD_MODE)
	params->dsi.mode                     = CMD_MODE;
	params->dsi.switch_mode              = SYNC_PULSE_VDO_MODE;
	#else
	params->dsi.mode                     = SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;//BURST_VDO_MODE;////
	#endif

	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
	params->dsi.LANE_NUM                 = LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order  = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq    = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding      = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format       = LCM_DSI_FORMAT_RGB888;

	params->dsi.PS                       = LCM_PACKED_PS_24BIT_RGB888;

	#if (LCM_DSI_CMD_MODE)
	params->dsi.intermediat_buffer_num   = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	params->dsi.word_count               = FRAME_WIDTH * 3; //DSI CMD mode need set these two bellow params, different to 6577
	#else
	params->dsi.intermediat_buffer_num   = 2;	//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	#endif
	// Video mode setting
	params->dsi.packet_size              = 256;

	params->dsi.vertical_sync_active     = 2;
	params->dsi.vertical_backporch       = 12;
	params->dsi.vertical_frontporch      = 186;
	params->dsi.vertical_active_line     = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active   = 9;
	params->dsi.horizontal_backporch     = 29;//36
	params->dsi.horizontal_frontporch    = 17;//78
	params->dsi.horizontal_active_pixel  = FRAME_WIDTH;
	/* params->dsi.ssc_disable = 1; */
	params->dsi.PLL_CLOCK                = 272;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9d;
}



/*static unsigned int lcm_compare_id(void)
{
    #define LCM_ID 0x8310
    int array[4];
    char buffer[5];
    int id = 0;
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
    array[0] = 0x00023700; // read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0x04, buffer, 2);
    id = (buffer[0] << 8) | buffer[1]; 
#ifdef BUILD_LK
    printf("lsw: hx83102d %s %d, id = 0x%08x\n", __func__,__LINE__, id);
#else
    printk("lsw: hx83102d %s %d, id = 0x%08x\n", __func__,__LINE__, id);
#endif
    return (id == LCM_ID) ? 1 : 0;
}
*/

#define AUXADC_LCM_VOLTAGE_CHANNEL (2)
#define MIN_VOLTAGE (900000)
#define MAX_VOLTAGE (1100000)
extern int IMM_GetOneChannelValue_Cali(int Channel, int *voltage);
static unsigned int hx_lcm_compare_id(void)
{
    int res = 0;
    int lcm_vol = 0;
#ifdef AUXADC_LCM_VOLTAGE_CHANNEL
    res = IMM_GetOneChannelValue_Cali(AUXADC_LCM_VOLTAGE_CHANNEL,&lcm_vol);
    if(res < 0)
    {
#ifdef BUILD_LK
        dprintf(0,"lixf lcd [adc_uboot]: get data error\n");
#endif
        return 0;
    }
#endif
#ifdef BUILD_LK
    dprintf(0,"lsw lk: lcm_vol= %d , file : %s, line : %d\n",lcm_vol, __FILE__, __LINE__);
#else
    printk("lsw kernel: lcm_vol= %d , file : %s, line : %d\n",lcm_vol, __FILE__, __LINE__);
#endif
    if (lcm_vol >= MIN_VOLTAGE && lcm_vol <= MAX_VOLTAGE)
    {
        return 1;
    }


    return 0;
}



static void lcm_init(void)
{
	int ret = 0;
	ret = PMU_REG_MASK(0xB2, (0x3 << 6), (0x3 << 6));
	ret = PMU_REG_MASK(0xB3, 28, (0x3F << 0));
	ret = PMU_REG_MASK(0xB4, 28, (0x3F << 0));
	ret = PMU_REG_MASK(0xB1, (1<<3) | (1<<6), (1<<3) | (1<<6));
	MDELAY(10);
    MDELAY(5);
    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(5);
    SET_RESET_PIN(1);
    MDELAY(10);
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	int ret = 0;
    push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);

    SET_RESET_PIN(1);
    MDELAY(5);
	ret = PMU_REG_MASK(0xB1, (0<<3) | (0<<6), (1<<3) | (1<<6));
	MDELAY(5);
}

static void lcm_resume(void)
{
    lcm_init();
}

LCM_DRIVER hx83102d_hdp_dsi_vdo_boe_drip_incell_lcm_drv =
{
    .name			= "hx83102d_hdp_dsi_vdo_boe_drip_incell",
#if defined(CONFIG_PRIZE_HARDWARE_INFO) && !defined (BUILD_LK)
	.lcm_info = {
		.chip	= "hx83102d",
		.vendor	= "focaltech",
		.id		= "0x82",
		.more	= "720*1600",
	},
#endif
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = hx_lcm_compare_id,
};


#include <osl.h>
#include <dhd_linux.h>

#ifdef CONFIG_PLAT_AMBARELLA
#define WIFI_DRIVER_NAME "bcmdhd"

#include <config.h>
#include <linux/gpio.h>
#include <plat/sd.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)
#include <mach/board.h>
#endif
#endif

#ifdef CONFIG_MACH_ODROID_4210
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/sdhci.h>
#include <plat/devs.h>	// modifed plat-samsung/dev-hsmmcX.c EXPORT_SYMBOL(s3c_device_hsmmcx) added

#define	sdmmc_channel	s3c_device_hsmmc0
#endif

#ifdef CONFIG_PLAT_AMBARELLA
int amba_gpio_request(int gpio_id)
{
	int ret = -1;
	ret = gpio_request(GPIO(gpio_id), WIFI_DRIVER_NAME);
	if (ret < 0) {
		printk("gpio request (%d) failed.\n", gpio_id);
		return ret;
	}
	return ret;
}

int amba_gpio_output(int gpio_id, int value)
{
	return gpio_direction_output(GPIO(gpio_id), value);;
}

int amba_gpio_irq(int gpio_id, int *irq_num)
{
	*irq_num = gpio_to_irq(GPIO(gpio_id));
	return gpio_direction_input(GPIO(gpio_id));
}

int amba_gpio_free(int gpio_id)
{
	gpio_free(GPIO(gpio_id));
	return 0;
}
#endif

struct wifi_platform_data dhd_wlan_control = {0};

#ifdef CUSTOMER_OOB
uint bcm_wlan_get_oob_irq(void)
{
	uint host_oob_irq = 0;

#ifdef CONFIG_MACH_ODROID_4210
	printk("GPIO(WL_HOST_WAKE) = EXYNOS4_GPX0(7) = %d\n", EXYNOS4_GPX0(7));
	host_oob_irq = gpio_to_irq(EXYNOS4_GPX0(7));
	gpio_direction_input(EXYNOS4_GPX0(7));
#endif

#ifdef CONFIG_PLAT_AMBARELLA
	printk("GPIO(WL_HOST_WAKE) [%d]\n", GPIO_BCM_WL_HOST_WAKE);
	amba_gpio_request(GPIO_BCM_WL_HOST_WAKE);
	amba_gpio_irq(GPIO_BCM_WL_HOST_WAKE, &host_oob_irq);
#endif

	printk("host_oob_irq: %d \r\n", host_oob_irq);

	return host_oob_irq;
}

uint bcm_wlan_get_oob_irq_flags(void)
{
	uint host_oob_irq_flags = 0;

#ifdef CONFIG_MACH_ODROID_4210
	host_oob_irq_flags = (IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE) & IRQF_TRIGGER_MASK;
#endif

#ifdef CONFIG_PLAT_AMBARELLA
	host_oob_irq_flags = (IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE) & IRQF_TRIGGER_MASK;
#endif

	printk("host_oob_irq_flags=%d\n", host_oob_irq_flags);

	return host_oob_irq_flags;
}
#endif

int bcm_wlan_set_power(bool on)
{
	int err = 0;

	if (on) {
		printk("======== PULL WL_REG_ON[%d], active[%d], HIGH! ========\n",
			GPIO_BCM_WL_REG_ON, GPIO_BCM_WL_REG_ON_ACTIVE);

#ifdef CONFIG_MACH_ODROID_4210
		err = gpio_set_value(EXYNOS4_GPK1(0), 1);
#endif

#ifdef CONFIG_PLAT_AMBARELLA
		amba_gpio_request(GPIO_BCM_WL_REG_ON);
		amba_gpio_output(GPIO_BCM_WL_REG_ON, GPIO_BCM_WL_REG_ON_ACTIVE);
#endif
		/* Lets customer power to get stable */
		mdelay(100);
	} else {
		printk("======== PULL WL_REG_ON[%d], inactive[%d], LOW! ========\n",
			GPIO_BCM_WL_REG_ON, !GPIO_BCM_WL_REG_ON_ACTIVE);

#ifdef CONFIG_MACH_ODROID_4210
		err = gpio_set_value(EXYNOS4_GPK1(0), 0);
#endif

#ifdef CONFIG_PLAT_AMBARELLA
		amba_gpio_output(GPIO_BCM_WL_REG_ON, !GPIO_BCM_WL_REG_ON_ACTIVE);
		amba_gpio_free(GPIO_BCM_WL_REG_ON);
#ifdef CUSTOMER_OOB
		amba_gpio_free(GPIO_BCM_WL_HOST_WAKE);
#endif
#endif
	}

	return err;
}

int bcm_wlan_set_carddetect(bool present)
{
	int err = 0;

	if (present) {
		printk("======== Card detection to detect SDIO card! Slot.num[%d] ========\n",
			WIFI_CONN_SD_SLOT_NUM);
#ifdef CONFIG_MACH_ODROID_4210
		err = sdhci_s3c_force_presence_change(&sdmmc_channel, 1);
#endif

#ifdef CONFIG_PLAT_AMBARELLA
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 9, 0)
		if (WIFI_CONN_SD_SLOT_NUM >= 0 && WIFI_CONN_SD_SLOT_NUM < 5) {
			ambarella_detect_sd_slot(WIFI_CONN_SD_SLOT_NUM, 1);
		}
#else
		if (ambarella_board_generic.wifi_sd_bus >= 0 && ambarella_board_generic.wifi_sd_bus < 5) {
			ambarella_detect_sd_slot(ambarella_board_generic.wifi_sd_bus,
				ambarella_board_generic.wifi_sd_slot, 1);
		}
#endif
#endif
	} else {
		printk("======== Card detection to remove SDIO card! Slot.num[%d] =======\n",
			WIFI_CONN_SD_SLOT_NUM);
#ifdef CONFIG_MACH_ODROID_4210
		err = sdhci_s3c_force_presence_change(&sdmmc_channel, 0);
#endif

#ifdef CONFIG_PLAT_AMBARELLA
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 9, 0)
		if (WIFI_CONN_SD_SLOT_NUM >= 0 && WIFI_CONN_SD_SLOT_NUM < 5) {
			ambarella_detect_sd_slot(WIFI_CONN_SD_SLOT_NUM, 0);
		}
#else
		if (ambarella_board_generic.wifi_sd_bus >= 0 && ambarella_board_generic.wifi_sd_bus < 5) {
			ambarella_detect_sd_slot(ambarella_board_generic.wifi_sd_bus,
				ambarella_board_generic.wifi_sd_slot, 0);
		}
#endif
#endif
	}

	return err;
}

int bcm_wlan_get_mac_address(unsigned char *buf)
{
	int err = 0;

	printf("======== %s ========\n", __FUNCTION__);
#ifdef EXAMPLE_GET_MAC
	/* EXAMPLE code */
	{
		struct ether_addr ea_example = {{0x00, 0x11, 0x22, 0x33, 0x44, 0xFF}};
		bcopy((char *)&ea_example, buf, sizeof(struct ether_addr));
	}
#endif /* EXAMPLE_GET_MAC */

	return err;
}

#ifdef CONFIG_DHD_USE_STATIC_BUF
extern void *bcmdhd_mem_prealloc(int section, unsigned long size);
void* bcm_wlan_prealloc(int section, unsigned long size)
{
	void *alloc_ptr = NULL;
	alloc_ptr = bcmdhd_mem_prealloc(section, size);
	if (alloc_ptr) {
		printk("success alloc section %d, size %ld\n", section, size);
		if (size != 0L)
			bzero(alloc_ptr, size);
		return alloc_ptr;
	}
	printk("can't alloc section %d\n", section);
	return NULL;
}
#endif

#if !defined(WL_WIRELESS_EXT)
struct cntry_locales_custom {
	char iso_abbrev[WLC_CNTRY_BUF_SZ];	/* ISO 3166-1 country abbreviation */
	char custom_locale[WLC_CNTRY_BUF_SZ];	/* Custom firmware locale */
	int32 custom_locale_rev;		/* Custom local revisin default -1 */
};
#endif

static struct cntry_locales_custom brcm_wlan_translate_custom_table[] = {
	/* Table should be filled out based on custom platform regulatory requirement */
	{"",   "XT", 49},  /* Universal if Country code is unknown or empty */
	{"US", "US", 0},
};

static void *bcm_wlan_get_country_code(char *ccode)
{
	struct cntry_locales_custom *locales;
	int size;
	int i;

	if (!ccode)
		return NULL;

	locales = brcm_wlan_translate_custom_table;
	size = ARRAY_SIZE(brcm_wlan_translate_custom_table);

	for (i = 0; i < size; i++)
		if (strcmp(ccode, locales[i].iso_abbrev) == 0)
			return &locales[i];
	return NULL;
}

int bcm_wlan_set_plat_data(void) {
	printf("======== %s ========\n", __FUNCTION__);
	dhd_wlan_control.set_power = bcm_wlan_set_power;
	dhd_wlan_control.set_carddetect = bcm_wlan_set_carddetect;
	dhd_wlan_control.get_mac_addr = bcm_wlan_get_mac_address;
#ifdef CONFIG_DHD_USE_STATIC_BUF
	dhd_wlan_control.mem_prealloc = bcm_wlan_prealloc;
#endif
	dhd_wlan_control.get_country_code = bcm_wlan_get_country_code;
	return 0;
}


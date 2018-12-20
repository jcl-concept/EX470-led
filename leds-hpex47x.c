/*
 * LED Driver
 * For the HP EX47x MediaSmart Server
 *
 * Copyright 2009 David Kuder <dkuder@thewaffleiron.net>
 *     this driver is based on ledtrig-timer.c by Richard Purdie.
 *     this driver is also based on leds-hp6xx.c by Kristoffer Ericson.
 *     which itself is based on leds-spitz.c by Richard Purdie.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <asm/io.h>

#define DRIVER_NAME	"leds-hpex47x"
#define PFX		DRIVER_NAME ": "

#define DRIVER_DESC	"HP MediaSmart Server EX47x HDD Bay LED Driver"
#define DRIVER_AUTHOR	"David Kuder <dkuder@thewaffleiron.net>"

MODULE_AUTHOR("David Kuder <dkuder@thewaffleiron.net>");
MODULE_DESCRIPTION("HP MediaSmart Server EX47x HDD Bay LED Driver");
MODULE_LICENSE("GPL");

#define HPEX_PORT 0x1064
#define HPEX_CTRL 0xff7f

#define HPEX_LEDCOUNT 8

u8 HPEX_FRAME = 0;
static struct platform_device *pdev;

typedef void (*set_handler)(struct led_classdev *, enum led_brightness);
struct led_type {
        const char      *name;
        set_handler     handler;
        const char      *default_trigger;
        u16		bits;
};

struct hpex_led {
	struct led_classdev	led_cdev;
	int			index;
	u8			value;
	u16			bits;
};
#define	to_hpex_led(d) container_of(d, struct hpex_led, led_cdev)

struct hpex_drvdata {
        struct hpex_led		led[HPEX_LEDCOUNT];
	struct timer_list	timer;
};

static void hpex47x_led_update(unsigned long data)
{
	int i;
	struct hpex_drvdata *p = (struct hpex_drvdata *)data;
	struct hpex_led *led = p->led;
	
	u16 pbits = HPEX_CTRL;
	
	for(i=0; i<HPEX_LEDCOUNT; i++) {
		if((led[i].value>>5) > HPEX_FRAME) pbits &= led[i].bits;
	}
	
	HPEX_FRAME = (HPEX_FRAME + 1) & 0x7;

	outw(pbits, HPEX_PORT);

	mod_timer(&p->timer, jiffies + 1);
}

static void hpex47x_led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct hpex_led *p = to_hpex_led(led_cdev);
	p->value = value;
}

static struct led_type hpex47x_hdd_led[HPEX_LEDCOUNT] = {
	{
		.name			= "hpex47x:blue:hdd0",
		.handler		= hpex47x_led_set,
		.bits			= ~0x0001,
	}, {
		.name			= "hpex47x:blue:hdd1",
		.handler		= hpex47x_led_set,
		.bits			= ~0x0002,
	}, {
		.name			= "hpex47x:blue:hdd2",
		.handler		= hpex47x_led_set,
		.bits			= ~0x0008,
	}, {
		.name			= "hpex47x:blue:hdd3",
		.handler		= hpex47x_led_set,
		.bits			= ~0x0020,
	}, {
		.name			= "hpex47x:red:hdd0",
		.handler		= hpex47x_led_set,
		.bits			= ~0x1000,
	}, {
		.name			= "hpex47x:red:hdd1",
		.handler		= hpex47x_led_set,
		.bits			= ~0x0100,
	}, {
		.name			= "hpex47x:red:hdd2",
		.handler		= hpex47x_led_set,
		.bits			= ~0x0200,
	}, {
		.name			= "hpex47x:red:hdd3",
		.handler		= hpex47x_led_set,
		.bits			= ~0x0400,
		.default_trigger	= "heartbeat",
	}
};

static int __devinit hpex47x_led_probe(struct platform_device *pdev)
{
	struct hpex_drvdata *p;
	struct led_type *types = hpex47x_hdd_led;
	int i, err = -EINVAL;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p) {
		printk(KERN_ERR PFX "Could not allocate struct hpex_drvdata\n");
		goto out;
	}

	for (i = 0; i < HPEX_LEDCOUNT; i++) {
		struct led_classdev *lp = &p->led[i].led_cdev;

		p->led[i].index = i;
		p->led[i].bits = types[i].bits;
		lp->name = types[i].name;
		lp->brightness = LED_FULL;
		lp->brightness_set = types[i].handler;
		lp->default_trigger = types[i].default_trigger;

		err = led_classdev_register(&pdev->dev, lp);
		if (err) {
			printk(KERN_ERR PFX "Could not register %s LED\n",
			       lp->name);
			goto out_unregister_led_cdevs;
		}
	}

	init_timer(&p->timer);
	p->timer.function = hpex47x_led_update;
	p->timer.data = (unsigned long) p;
	dev_set_drvdata(&pdev->dev, p);

	mod_timer(&p->timer, jiffies + 1);

	err = 0;
out:
	return err;
	
out_unregister_led_cdevs:
	for (i--; i >= 0; i--)
		led_classdev_unregister(&p->led[i].led_cdev);
	goto out;
}

static int __devexit hpex47x_led_remove(struct platform_device *pdev)
{
	int i;

	struct hpex_drvdata *p = dev_get_drvdata(&pdev->dev);

	for (i = 0; i < HPEX_LEDCOUNT; i++) {
		led_classdev_unregister(&p->led[i].led_cdev);
	}

	del_timer_sync(&p->timer);

	kfree(p);

	// Turn off all leds
	outw(HPEX_CTRL, HPEX_PORT);

	return 0;
}

/* work with hotplug and coldplug */
MODULE_ALIAS("platform:hpex47x-led");

#define DRVNAME "hpex47x-led"

static struct platform_driver hpex47x_led_driver = {
	.probe		= hpex47x_led_probe,
	.remove		= __devexit_p(hpex47x_led_remove),
	.driver		= {
		.name		= DRVNAME,
		.owner		= THIS_MODULE,
	},
};

static int __init hpex47x_led_init(void)
{
	int err;
	
	printk(KERN_INFO PFX DRIVER_DESC "\n");
	printk(KERN_INFO PFX DRIVER_AUTHOR "\n");

	err = platform_driver_register(&hpex47x_led_driver);

	if (err) {
		printk(KERN_ERR PFX "Could not register HDD Status LED driver\n");
		goto out;
	}

	pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		err = PTR_ERR(pdev);
		platform_driver_unregister(&hpex47x_led_driver);
		goto out;
	}

out:
	return err;
}

static void __exit hpex47x_led_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&hpex47x_led_driver);
}

module_init(hpex47x_led_init);
module_exit(hpex47x_led_exit);

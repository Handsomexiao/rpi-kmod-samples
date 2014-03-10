/*
 * Basic Linux Kernel module using GPIO interrupts.
 *
 * Author:
 * 	Shuai Xiao (iamhihi@gmail.com)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h> 
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define TRIGGER_PIN 23
#define ECHO_PIN  24
#define TIMEOUT 999 /* any value other than LOW or HIGH */

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

/* Task handle to identify thread */
static struct task_struct *ts = NULL;

static volatile unsigned long start = 0;
static volatile unsigned long now = 0;
static volatile unsigned int done = 0;

//us
volatile unsigned long GetTimeStamp(void) {
    struct timeval tv;
    do_gettimeofday(&tv);
    return tv.tv_sec*(unsigned long)1000000+tv.tv_usec;
}

/* Define GPIOs for Trigger */
static struct gpio leds[] = {
		{ TRIGGER_PIN, GPIOF_OUT_INIT_LOW, "Trigger" },
};

/* Define GPIOs for Echo */
static struct gpio buttons[] = {
		{ ECHO_PIN, GPIOF_IN, "Echo" },
};

/* Later on, the assigned IRQ numbers for the buttons are stored here */
static int button_irqs[] = { -1};

static int led_thread(void *data)
{
	printk(KERN_INFO "%s\n", __func__);
    
    int pulsewidth;
    int timeInSecond;
    int distance;
    
    // loop until killed ...
    for( ; ; ) {
        if(kthread_should_stop()){
            return 0;
        }

        //# Set trigger to False (Low)
        gpio_set_value(leds[0].gpio, LOW);
        //# Allow module to settle
        mdelay(500);
        
        //# Send 10us pulse to trigger
        /* trigger reading */
        gpio_set_value(leds[0].gpio, HIGH);
        udelay(10);
        gpio_set_value(leds[0].gpio, LOW);
        
        while(done == 0){
            mdelay(100);
        }
        
        pulsewidth = abs(now - start);
        timeInSecond = (int)(pulsewidth * 34000 / 1000);
        distance = timeInSecond;
        distance = distance / 2;
        printk(KERN_INFO "Ping distance = %d mm\n", distance);
    }
    
	return 0;
}
/*
 * The interrupt service routine called on button presses
 */
static irqreturn_t button_isr(int irq, void *data)
{
    int newState;
    
    newState = gpio_get_value(buttons[0].gpio);
    
    if(newState == HIGH){
        done = 1;
        now =  GetTimeStamp();
    }
    else if(newState == LOW)
    {
        start = GetTimeStamp();
    }

	return IRQ_HANDLED;
}

/*
 * Module init function
 */
static int __init gpiomode_init(void)
{
	int ret = 0;

	printk(KERN_INFO "%s\n", __func__);

	// register LED gpios
	ret = gpio_request_array(leds, ARRAY_SIZE(leds));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs for LEDs: %d\n", ret);
		return ret;
	}
	
	// register BUTTON gpios
	ret = gpio_request_array(buttons, ARRAY_SIZE(buttons));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs for BUTTONs: %d\n", ret);
		goto fail1;
	}

	printk(KERN_INFO "Current button1 value: %d\n", gpio_get_value(buttons[0].gpio));
	
	ret = gpio_to_irq(buttons[0].gpio);

	if(ret < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	button_irqs[0] = ret;

	printk(KERN_INFO "Successfully requested BUTTON1 IRQ # %d\n", button_irqs[0]);

	ret = request_irq(button_irqs[0], button_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED, "gpiomod#button1", NULL);

	if(ret) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}
    
    ts = kthread_create(led_thread, NULL, "irqPing");
	if(ts) {
		wake_up_process(ts);
    }
	else {
		printk(KERN_ERR "Unable to create thread\n");
		goto fail3;
	}

	return 0;

// cleanup what has been setup so far
fail3:
	free_irq(button_irqs[0], NULL);

fail2: 
	gpio_free_array(buttons, ARRAY_SIZE(leds));

fail1:
	gpio_free_array(leds, ARRAY_SIZE(leds));

	return ret;	
}

/**
 * Module exit function
 */
static void __exit gpiomode_exit(void)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);

	// free irqs
    for(i = 0; i < ARRAY_SIZE(button_irqs); i++) {
		free_irq(button_irqs[0], NULL);
	}

	// unregister
	gpio_free_array(leds, ARRAY_SIZE(leds));
	gpio_free_array(buttons, ARRAY_SIZE(buttons));
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shuai Xiao");
MODULE_DESCRIPTION("Basic Linux Kernel module using GPIO interrupts");

module_init(gpiomode_init);
module_exit(gpiomode_exit);

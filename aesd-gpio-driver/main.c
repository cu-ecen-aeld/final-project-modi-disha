/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "aesd" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @modified by Disha Modi
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/slab.h>	
#include <linux/moduleparam.h>
#include <linux/uaccess.h>	
#include <linux/fcntl.h>
#include <linux/fs.h> // file_operations
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/string.h>
#include <linux/spinlock.h>

/* User-defined macros */
#define NUM_GPIO_PINS 21
#define MAX_GPIO_NUMBER 32
#define DEVICE_NAME "raspi-gpio"
#define BUF_SIZE 512

/* User-defined data types */
enum state {low, high};
enum direction {in, out};

struct raspi_gpio_dev {
	struct cdev cdev;
	struct gpio pin;
	enum state state;
	enum direction dir;
	spinlock_t lock;
};

static int raspi_gpio_open(struct inode *inode, struct file *filp);
static ssize_t raspi_gpio_read ( struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t raspi_gpio_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int raspi_gpio_release(struct inode *inode, struct file *filp);
static int raspi_gpio_init(void);
static void raspi_gpio_exit(void);

struct raspi_gpio_dev *raspi_gpio_devp[NUM_GPIO_PINS];
static dev_t first;
static struct class *raspi_gpio_class;

//int aesd_major =   0; // use dynamic major
//int aesd_minor =   0;

MODULE_AUTHOR("Disha Modi"); 
MODULE_LICENSE("Dual BSD/GPL");

//struct aesd_dev aesd_device;

/// aesd open routine
int raspi_gpio_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "open");

	struct raspi_gpio_dev *raspi_gpio_devp;
        unsigned int gpio; /* device information */

	gpio = iminor(inode);
	printk(KERN_INFO "GPIO[%d] opened\n", gpio);
	raspi_gpio_devp = container_of(inode->i_cdev, struct raspi_gpio_dev, cdev);
	filp->private_data = raspi_gpio_devp; /* for other methods */

	return 0;     
	/* success */
}

/// raspi_gpio_release routine
int raspi_gpio_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "release");
	return 0;
}

/// aesd read routine
ssize_t raspi_gpio_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	printk(KERN_INFO "read");
	unsigned int gpio;
	ssize_t retval;
	char byte;
	gpio = iminor(filp->f_path.dentry->d_inode);
	for (retval = 0; retval < count; ++retval) 
	{
		byte = '0' + gpio_get_value(gpio);
		if(put_user(byte, buf+retval))
		break;
	}

	printk(KERN_INFO "Read retval %ld",retval);
	return retval;
}

/// aesd write routine
ssize_t raspi_gpio_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	unsigned int gpio, len = 0, value = 0;
	char kbuf[BUF_SIZE];
	struct raspi_gpio_dev *raspi_gpio_devp = filp->private_data;
	unsigned long flags;
	gpio = iminor(filp->f_path.dentry->d_inode);
	len = count < BUF_SIZE ? count-1 : BUF_SIZE-1;
	if(copy_from_user(kbuf, buf, len) != 0)
	return -EFAULT;
	kbuf[len] = '\0';
	printk(KERN_INFO "Request from user: %s\n", kbuf);
	// Check the content of kbuf and set GPIO pin accordingly
	if (strcmp(kbuf, "out") == 0) 
	{
		printk(KERN_ALERT "gpio[%d] direction set to ouput\n", gpio);
		if (raspi_gpio_devp->dir != out) {
		spin_lock_irqsave(&raspi_gpio_devp->lock, flags);
		gpio_direction_output(gpio, low);
		raspi_gpio_devp->dir = out;
		raspi_gpio_devp->state = low;
		spin_unlock_irqrestore(&raspi_gpio_devp->lock, flags);
		}
	} else if (strcmp(kbuf, "in") == 0) {
		if (raspi_gpio_devp->dir != in) {
		printk(KERN_INFO "Set gpio[%d] direction: input\n", gpio);
		spin_lock_irqsave(&raspi_gpio_devp->lock, flags);
		gpio_direction_input(gpio);
		raspi_gpio_devp->dir = in;
		spin_unlock_irqrestore(&raspi_gpio_devp->lock, flags);
		}
	} else if ((strcmp(kbuf, "1") == 0) || (strcmp(kbuf, "0") == 0)) 
	{
		sscanf(kbuf, "%d", &value);
		if (raspi_gpio_devp->dir == in) {
			printk("Cannot set GPIO %d, direction: input\n", gpio);
			return -EPERM;
		}
		if (raspi_gpio_devp->dir == out) {
			if (value > 0) {
			spin_lock_irqsave(&raspi_gpio_devp->lock, flags);
			gpio_set_value(gpio, high);
			raspi_gpio_devp->state = high;
			spin_unlock_irqrestore(&raspi_gpio_devp->lock, flags);
			} else {
			spin_lock_irqsave(&raspi_gpio_devp->lock, flags);
			gpio_set_value(gpio, low);
			raspi_gpio_devp->state = low;
			spin_unlock_irqrestore(&raspi_gpio_devp->lock, flags);
			}
		}
	} else if ( (strcmp(kbuf, "rising") == 0) || (strcmp(kbuf, "falling") == 0)) 
	{
		spin_lock_irqsave(&raspi_gpio_devp->lock, flags);
		gpio_direction_input(gpio);
		raspi_gpio_devp->dir = in;
		spin_unlock_irqrestore(&raspi_gpio_devp->lock, flags);
	} else 
	{
		printk(KERN_ERR "Invalid value\n");
		return -EINVAL;
	}

	*f_pos += count;
	return count;
}

static struct file_operations raspi_gpio_fops = {
	.owner = THIS_MODULE,
	.open = raspi_gpio_open,
	.release = raspi_gpio_release,
	.read = raspi_gpio_read,
	.write = raspi_gpio_write,
};

int raspi_gpio_init(void)
{
	int i, ret, index = 0;
	if (alloc_chrdev_region(&first, 0, NUM_GPIO_PINS, DEVICE_NAME) < 0) {
		printk(KERN_DEBUG "Cannot register device\n");
		return -1;
	}
	if ((raspi_gpio_class = class_create( THIS_MODULE, DEVICE_NAME)) == NULL) {
		printk(KERN_DEBUG "Cannot create class %s\n", DEVICE_NAME);
		unregister_chrdev_region(first, NUM_GPIO_PINS);
		return -EINVAL;
	}

	for (i = 0; i < MAX_GPIO_NUMBER; i++) {
		if ( i != 0 && i != 1 && i != 5 && i != 6 &&
			i != 12 && i != 13 && i != 16 && i != 19 &&
			i != 20 && i != 21 && i != 26) 
		{
			raspi_gpio_devp[index] = kmalloc(sizeof(struct raspi_gpio_dev), GFP_KERNEL);
			if (!raspi_gpio_devp[index]) {
				printk("Bad kmalloc\n");
				return -ENOMEM;
			}
			if (gpio_request_one(i, GPIOF_OUT_INIT_LOW, NULL) < 0) {
			printk(KERN_ALERT "Error requesting GPIO %d\n", i);
			return -ENODEV;
			}

			raspi_gpio_devp[index]->dir = out;
			raspi_gpio_devp[index]->state = low;
			raspi_gpio_devp[index]->cdev.owner = THIS_MODULE;
			spin_lock_init(&raspi_gpio_devp[index]->lock);
			cdev_init(&raspi_gpio_devp[index]->cdev, &raspi_gpio_fops);

			if ((ret = cdev_add( &raspi_gpio_devp[index]->cdev, (first + i), 1))) {
				printk (KERN_ALERT "Error %d adding cdev\n", ret);
				for (i = 0; i < MAX_GPIO_NUMBER; i++) {
					if ( i != 0 && i != 1 && i != 5 && i != 6 &&
					i != 12 && i != 13 && i != 16 && i != 19 &&
					i != 20 && i != 21 && i != 26) {
					device_destroy (raspi_gpio_class, MKDEV(MAJOR(first), MINOR(first) + i));
					}

				}
				class_destroy(raspi_gpio_class);
				unregister_chrdev_region(first, NUM_GPIO_PINS);
				return ret;
			}
			if (device_create( raspi_gpio_class, NULL, MKDEV(MAJOR(first), MINOR(first)+i), NULL, "raspiGpio%d", i) == NULL) 
			{
				class_destroy(raspi_gpio_class);
				unregister_chrdev_region(first, NUM_GPIO_PINS);
				return -1;
			}
			index++;
		}
	}

	printk("RaspberryPi GPIO driver initialized\n");
	return 0;
}		

void raspi_gpio_exit(void)
{
	int i = 0;
	unregister_chrdev_region(first, NUM_GPIO_PINS);
	for (i = 0; i < NUM_GPIO_PINS; i++)
		kfree(raspi_gpio_devp[i]);
	for (i = 0; i < MAX_GPIO_NUMBER; i++) {
		if ( i != 0 && i != 1 && i != 5 && i != 6 &&
		i != 12 && i != 13 && i != 16 && i != 19 &&
		i != 20 && i != 21 && i != 26) {
			gpio_direction_output(i, 0);
			device_destroy ( raspi_gpio_class,
			MKDEV(MAJOR(first), MINOR(first) + i));
			gpio_free(i);
		}
	}
	class_destroy(raspi_gpio_class);
	printk(KERN_INFO "RaspberryPi GPIO driver removed\n");
}



module_init(raspi_gpio_init);
module_exit(raspi_gpio_exit);

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
#include "aesdchar.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Disha Modi"); 
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

/// aesd open routine
int aesd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("open");

	struct aesd_dev *dev; /* device information */

	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev; /* for other methods */

	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (mutex_lock_interruptible(&dev->lock))
			return -ERESTARTSYS;
		mutex_unlock(&dev->lock);
	}
	return 0;     
	/* success */
}

/// aesd release routine
int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	return 0;
}

/// aesd read routine
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	PDEBUG("read");
	struct aesd_dev *dev = filp->private_data; 
	ssize_t buff_offs = 0;
	ssize_t retval = 0;
	ssize_t bytespresent = 0;
	const char *local_buff;
	struct aesd_buffer_entry *locentry;

	if (mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;
	
	/// find entry and offset based on *f_pos
	locentry = aesd_circular_buffer_find_entry_offset_for_fpos(aesd_device.buffer, *f_pos, &buff_offs);
	if (locentry == NULL)
	{
	    goto out; 
	}

	/// check the number of counts requested against bytes present in the entry
	bytespresent = locentry->size - buff_offs;
	if(bytespresent < count)
	{
		count = bytespresent;
	}

        /// copy to user buf
	local_buff = locentry->buffptr;
	if (copy_to_user(buf, ((locentry->buffptr) + buff_offs), count)) {
		retval = -EFAULT;
		goto out;
	}
	
	*f_pos += count;
	retval = count;
	PDEBUG("End read");
  out:
	mutex_unlock(&dev->lock);
	PDEBUG("Read retval %ld",retval);
	return retval;
}

/// aesd write routine
ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	PDEBUG("write %zu bytes with offset %lld",count, *f_pos);
	static char *wbuff;
	struct aesd_dev *dev = filp->private_data;
	ssize_t buff_end = 0;
	const char *retbuff;
	static ssize_t posvar = 0;	
	struct aesd_buffer_entry *locentry = aesd_device.lentry;
	static bool flag = false;
	
	/// Allocate local buffer
	char *local_buf = (char *)kmalloc(count+1 * sizeof(char), GFP_KERNEL);
	if (!local_buf)
		goto out;
	memset(local_buf, 0, count * sizeof(char));
	
	
	if (mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;

	/// Copy user buf to the local buffer
	if (copy_from_user(local_buf, buf, count)) {
		kfree(local_buf);
		retval = -EFAULT;
		goto out;
	}

	/// check null character
	if(local_buf[count-1] == '\n')
	{
		if(!flag)
		{	
			wbuff = (char *)kmalloc(count+1 * sizeof(char), GFP_KERNEL);
			if(!wbuff)
			{
				goto out;
			}
			flag = true;
		}
		else
		{
			krealloc((char *)wbuff, (count+posvar) * sizeof(char), GFP_KERNEL);
			if(!wbuff)
			{
				goto out;
			}
		}
	
		memcpy(wbuff+posvar, local_buf, count);
		locentry->buffptr = wbuff;
		locentry->size = posvar+count;

		/// Add to the circular buffer
		retbuff = aesd_circular_buffer_add_entry(aesd_device.buffer, locentry);
		posvar = 0;
		if(retbuff)
		{
			kfree(retbuff);
		}	
		wbuff = NULL;
		flag = false;
		
	}
	else
	{
		if(!flag)
		{	
			wbuff = (char *)kmalloc(count+1 * sizeof(char), GFP_KERNEL);
			if(!wbuff)
			{
				goto out;
			}
			flag = true;
		}
		else
		{
			krealloc((char *)wbuff, (count+posvar) * sizeof(char), GFP_KERNEL);
			if(!wbuff)
			{
				goto out;
			}
		}	
	
		memcpy(wbuff+posvar, local_buf, count);
		posvar += count;
	}

	kfree(local_buf);
	*f_pos += count;
	retval = count;

	out:
		mutex_unlock(&dev->lock);
		return retval;
}

struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1, "aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));
	aesd_device.buffer = (struct aesd_circular_buffer *)kmalloc(1*sizeof(struct aesd_circular_buffer), GFP_KERNEL);
	memset(aesd_device.buffer,0,sizeof(struct aesd_circular_buffer));
	aesd_device.buffer->in_offs = 0;
	aesd_device.buffer->out_offs = 0;
	aesd_device.buffer->full = false;
	aesd_device.lentry = (struct aesd_buffer_entry *)kmalloc(1*sizeof(struct aesd_buffer_entry), GFP_KERNEL);
	memset(aesd_device.lentry,0,sizeof(struct aesd_buffer_entry));
	mutex_init(&aesd_device.lock);
	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	return result;
}

void aesd_cleanup_module(void)
{
	dev_t devno = MKDEV(aesd_major, aesd_minor);
	kfree(aesd_device.lentry);
	kfree(aesd_device.buffer);
	mutex_destroy(&aesd_device.lock);
	cdev_del(&aesd_device.cdev);
	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

/*
 * aesdi2c.h
 *
 *  Created on: April 12, 2021
 *      Author: Disha Modi
 */

#ifndef AESD_I2C_DRIVER_AESDI2C_H_
#define AESD_I2C_DRIVER_AESDI2C_H_

#include "aesd-i2c-drivers.h"
#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

struct aesdi2c_dev
{
	struct aesd_circular_buffer *buffer;
	struct aesd_buffer_entry *lentry;
	struct mutex lock;
	struct cdev cdev;	  /* Char device structure		*/
};


#endif /* AESD_I2C_DRIVER_AESDI2C_H_ */

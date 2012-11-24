#include <linux/module.h>
#include <linux/fs.h>		/* for file_operations */
#include <linux/version.h>	/* versioning */
#include <linux/cdev.h>

#define MODULE_NAME "OFFSHCED"

static int    devices_count  = 2;
static dev_t  device_number;
static struct cdev cdev; 
static int offline_cpuid = 1;

extern void register_offsched(void (*callback)(void), int cpuid);
extern void unregister_offsched(int cpuid);

/*
 * this code regisers offsched callback in offline_cpuid processor. 
 *  to use it please :
 *  insmod it , with module parameters cpuid=1 or any other cpu id other than 0
 *  from command line:
 *   echo 0 > /sys/devices/system/cpu/cpu<cpuid>/online
 *    offshced_main is called as an offlet.
*/

void offsched_main(void)
{	
	printk("offsched main cpu= %d\n",raw_smp_processor_id());	
}

/* return number of bytes done , negative value for error  */
int driver_open(struct inode *inode, struct file *filp)
{
	printk(MODULE_NAME KERN_INFO "open\n");
	return 0;
}

/* return number of bytes done , negative value for error  */
static ssize_t driver_write(struct file *filp,
	const char __user *umem, size_t size, loff_t *off)
{
	printk(MODULE_NAME KERN_INFO "write\n");
	return -1;
}

static ssize_t driver_read(struct file *filp, char __user *umem, 
				size_t size, loff_t *off)
{
	printk(MODULE_NAME KERN_INFO "read \n");
	return -1;
}

int driver_close(struct inode *inode, struct file *filp)
{
	printk(MODULE_NAME KERN_INFO "close \n");
	return 0;
}

struct file_operations driver_ops = {
	open: driver_open,
	write: driver_write,
	read:  driver_read,
	release: driver_close
};


void offsched_cleanup(void) 
{
	unregister_offsched(offline_cpuid);
	printk(MODULE_NAME KERN_INFO "exit\n");
	cdev_del(&cdev);
}

int offsched_init(void)
{
	int ret;
	int base_minor = 0;
	int index = 1;

	printk(MODULE_NAME KERN_INFO "init\n");

        if (alloc_chrdev_region(&device_number,
                   base_minor, devices_count, "offsched")) {
            return EBUSY;
	}

	cdev_init(&cdev, &driver_ops);
  	cdev.owner = THIS_MODULE;
	ret = cdev_add(&cdev,
            MKDEV(MAJOR(device_number), index), devices_count);
	if (ret < 0 ){
		printk("Failed to cdev_add\n");
		return -1;
	}
	register_offsched(offsched_main, offline_cpuid);
	return ret;
}


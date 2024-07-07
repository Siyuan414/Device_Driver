#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "char device"
#define BUF_LEN 80

static int major;
static char msg[BUF_LEN];
static struct cdev char_cdev;

static int dev_open(struct inode *inode, struct file*file){
	printk(KERN_INFO "char_device: opened\n");
	return 0;
}


static int dev_release(struct inode *inode, struct file*file){
	printk(KERN_INFO "char_device: closed\n");
	return 0;
}

static ssize_t dev_read(struct file *flip, char __user *buffer, size_t length, loff_t *offset){
	int bytes_read =0;
	char *msg_ptr = msg; 
	if(*msg_ptr ==0){
		return 0;
	}

	msg_ptr += *offset; 

	while(length && *msg_ptr){
		if(put_user(*(msg_ptr++),buffer++)){
			return -EFAULT;
		}
		
		length--;
		bytes_read++;
	}

	*offset += bytes_read;
	printk(KERN_INFO "char_device: read %d bytes\n",bytes_read);
	return bytes_read;
}

static ssize_t dev_write(struct file *flip, const char __user *buffer, size_t length, loff_t *offset){
	int bytes_written =0;
	if(length > BUF_LEN -1){
		length = BUF_LEN - 1;
	}
	if(copy_from_user(msg,buffer,length) != 0){
		return -EFAULT;
	}

	msg[length] = '\0';
	bytes_written = length;
	printk(KERN_INFO "char_device: wrote %d bytes\n",bytes_written);
	return bytes_written;
}

static struct file_operations fops = {
	.read = dev_read,
	.write = dev_write,
	.open = dev_open,
	.release = dev_release,
};

static int __init char_init(void){
	dev_t dev;
	int result;
	
	result = alloc_chrdev_region(&dev,0,1,DEVICE_NAME);
	if(result > 0){
		printk(KERN_WARNING "char_device: can't get major number\n");
		return result;
	}

	major = MAJOR(dev);
	cdev_init(&char_cdev, &fops);
	char_cdev.owner = THIS_MODULE;
	result  =  cdev_add(&char_cdev,dev,1);
	if(result){
		unregister_chrdev_region(dev,1);
		printk(KERN_WARNING "char_device: can't add cdev\n");
		return result;
	}
	printk(KERN_INFO "char_device: registered with major number %d\n",major);
	return 0;
}

static void __exit char_exit(void){
	cdev_del(&char_cdev);
	unregister_chrdev_region(MKDEV(major,0),1);
	printk(KERN_INFO,"char_device: unregistered\n");
}

module_init(char_init);
module_exit(char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SZ");
MODULE_DESCRIPTION("SIMPLE CHAR DIRVER");





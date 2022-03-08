#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "mfrc522.h"
#include "mfrc522_emu.h"

/*
 * Metadonnée
 */

MODULE_AUTHOR("TEIXEIRA ESTEVES Alexandre");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Driver Gistre Card");

/*
 * Local definitions
 */

#define GISTRE_CARD_BUFSIZE 25 // 25 (word max size)
#define MAXIMUM_BUFSIZE 38 // 9 + 1 + 2 + 1 + 25 (word max size)
struct gistre_card_dev {
	struct cdev cdev;
	struct regmap *map;
	size_t c_len;
};

/* Major will always be dynamically allocated */
static int major;
static struct gistre_card_dev *gc_dev;


/*
 * Function use during write command
 */

int len_of(char *str) // return the length of a char
{
	int i = 0;

	while(str[i] != '\0') 
		i++;
	return i + 1;
}
static char **split(char *str, size_t len) // split when ':'
{
	char copy[len + 1];
	memset(copy, 0, len + 1);
	memcpy(copy, str, len);
	int i = 0;
	char *tmp = kmalloc((25) * sizeof(char), GFP_KERNEL);
	int tmp_i = 0;

	char **res = kmalloc(3 * sizeof(char *), GFP_KERNEL);
	
	int n = 0;
	while (n < 3)
		res[n++] = NULL;

	int res_i = 0;

	while (copy[i] != '\0'/* && tmp_i < 25*/) {
		if (copy[i] != ':') {
			tmp[tmp_i++] = copy[i];
		}
		else {
			tmp[tmp_i] = '\0';
			size_t a = len_of(tmp);
			char *word = kmalloc(a * sizeof(char), GFP_KERNEL);
			strncpy(word, tmp, a);
			tmp_i = 0;
			res[res_i++] = word;
		}
		i++;
	}
	tmp[tmp_i] = '\0';
	size_t a = len_of(tmp);
	char *word = kmalloc(a * sizeof(char), GFP_KERNEL);
	strncpy(word, tmp, a);
	res[res_i++] = word;
	
	kfree(tmp);
	
	return res;
}

/*
 * Character device filed operations
 */

int gistre_card_open(struct inode *inode, struct file *file) 
{

	unsigned int i_major = imajor(inode);
	unsigned int i_minor = iminor(inode);

	// pr_info("%s()\n", __func__);

	if (i_major != major) {
		pr_err("Invalid major %d, expected %d\n", i_major, major);
		return -EINVAL;
	}
	if (i_minor != 0) {
		pr_err("pingpong driver only handles minor 0!\n");
		return -EINVAL;
	}

	file->private_data = gc_dev;
	
	return 0;
}

int gistre_card_release(struct inode *inode /* unused */,
                        struct file *file /* unused */) 
{
	// pr_info("%s()\n", __func__);
	/* Nothing in particular to do */
	return 0;
}

ssize_t gistre_card_read(struct file *file, char __user *buf, size_t len,
                         loff_t *off /* unused */) 
{
	struct gistre_card_dev *dev;
	dev = (struct gistre_card_dev *)file->private_data;
	return dev->c_len;
}




















ssize_t gistre_card_write(struct file *file, const char __user *buf, size_t len, 
                          loff_t *off /* unused */) 
{
	/*
	 *  treatment of the command writed 
	 */

	// Definition dev & input_buffer
	struct gistre_card_dev *dev;
	int ret;

	// Récupération dev
	dev = (struct gistre_card_dev *)file->private_data;

	
	// len : taille de command
  
	/* 
	if (MAXIMUM_BUFSIZE < len) {
		pr_info("[cmd] : mem_cmd can read only 25 bytes");
		return -EFAULT;
	}
	*/
	len = min(MAXIMUM_BUFSIZE, len);

	char input_buffer[len + 1];
	memset(input_buffer, 0, len+1);
	if (copy_from_user(input_buffer, buf, len)) {
		pr_err("Failed to copy user data\n");
		return -EFAULT;
	}
	char **splited_command = split(input_buffer,len);
  
	/*
	 * treatment of the content
	 */

	if (strcmp(splited_command[0], "mem_read") == 0) {
    
		/*
 		 * Command mem_read
		 */
   
    
		ret = regmap_write(dev->map, MFRC522_CMDREG, MFRC522_MEM);
		if (ret < 0) {
			pr_info("[cmd_read] : Error command MEM_READ in using the map.");
			return -EFAULT;
		}
    
		len = dev->c_len;
		char str_out[len + 1];
		memset(str_out, 0, len + 1);
		unsigned int ui_out;

		size_t index = 0;
		while (index < len) {
			ret = regmap_read(dev->map, MFRC522_FIFODATAREG, &ui_out);
			if (ret < 0) {
				pr_info("[fifo_read] : Error command MEM_READ in using the map.");
				return -EFAULT;
			}
			char c_out = (char)ui_out;
			str_out[index++] = c_out;
   
		}
		if (copy_to_user(buf, str_out, len)) {
			pr_err("Failed to copy data to user\n");
			return -EFAULT;
		}
	} else if (strcmp(splited_command[0], "mem_write") == 0) {

		/*
	 	 * Command mem_write
		 */
		char *word = splited_command[2];

		unsigned long tmp;
		kstrtoul(splited_command[1], 0, &tmp);
		len = tmp;
		len = min(tmp, GISTRE_CARD_BUFSIZE);
    
    /*
    if(len > GISTRE_CARD_BUFSIZE){
    pr_err("[mem_cmd] : The argument 'size' is invalid");
    return -EFAULT;
    }
    */

		dev->c_len = len;
		char gc_in_buf[GISTRE_CARD_BUFSIZE + 1];
		memset(gc_in_buf, 0, GISTRE_CARD_BUFSIZE + 1);
		memcpy(gc_in_buf, word, len);

		size_t i;
		while (i < GISTRE_CARD_BUFSIZE) {
			ret = regmap_write(dev->map, MFRC522_FIFODATAREG, gc_in_buf[i]);
			if (ret < 0) {
				pr_info("[fifo_write] : Error command MEM_WRITE in using the map.");
				return -EFAULT;
			}
			i++;
		}

		ret = regmap_write(dev->map, MFRC522_CMDREG, MFRC522_MEM);
		if (ret < 0) {
			pr_info("[cmd_write] : Error command MEM_WRITE in using the map.");
			return -EFAULT;
		}
	} else {
		return -EFAULT;
	}

	return GISTRE_CARD_BUFSIZE;
}




/*
 *  Gistre Card fops, creation & destroy
 */

static struct file_operations gistre_card_fops = {
	.owner = THIS_MODULE,
	.open = gistre_card_open,
	.release = gistre_card_release,
	.read = gistre_card_read,
	.write = gistre_card_write,
    /* Others functions are using the kernel's defaults */
};

static struct gistre_card_dev *gistre_card_create(void) 
{
	struct gistre_card_dev *dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) 
		return NULL;

	dev->cdev.owner = THIS_MODULE;
	cdev_init(&dev->cdev, &gistre_card_fops);

	struct device *my_device;
	my_device = mfrc522_find_dev();

	struct mfrc522_dev *my_mfrc522_dev;
	my_mfrc522_dev = dev_to_mfrc522(my_device);

	struct regmap *map;
	map = mfrc522_get_regmap(my_mfrc522_dev);
	dev->map = map;

	dev->c_len = 0;

	return dev;
}

static void gistre_card_destroy(struct gistre_card_dev *dev) 
{
	cdev_del(&dev->cdev);
	kfree(dev);
}

/*
 * Init & Exit
 */

__exit static void gistre_card_exit(void) 
{
	dev_t dev;

	dev = MKDEV(major, 0);
	gistre_card_destroy(gc_dev);
	unregister_chrdev_region(dev, 1);
}

__init static int gistre_card_init(void) {

  /*
   * Palier 1
   */

	pr_info("Hello, GISTRE card!\n");

  /*
   * Palier 2
   */

	dev_t dev;
	int ret;
	const char devname[] = "gistre_card";

  /* Allocate major */
	ret = alloc_chrdev_region(&dev, 0, 1, devname);
	if (ret < 0) {
		pr_info("Failed to allocate major\n");
		return 1;
	} else {
		major = MAJOR(dev);
		pr_info("Got major %d\n", major);
	}

  /* Register char device */
	gc_dev = gistre_card_create();
	if (!gc_dev) {
		pr_err("Failed to init gistre_card_dev\n");
		return -ENOMEM;
	}

	if (cdev_add(&gc_dev->cdev, dev, 1) < 0) {
		pr_err("Failed to register char device\n");
		return -ENOMEM;
	}

	return 0;
}

MODULE_SOFTDEP("pre: mfrc522_emu");

module_init(gistre_card_init);
module_exit(gistre_card_exit);

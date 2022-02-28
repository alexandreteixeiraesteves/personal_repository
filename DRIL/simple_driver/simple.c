#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

/*
 * Metadonnée 
 */

MODULE_AUTHOR("TEIXEIRA ESTEVES Alexandre");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Driver Gistre Card");


/*
 * Local definitions
 */

#define GISTRE_CARD_BUFSIZE 25
#define MAXIMUM_BUFSIZE 38  // input maximum valide est 9 char (mem_write) + 1 (:) + 2 (int) + 1 (:) + 25 (word max size)

struct gistre_card_dev {
	struct cdev cdev;
	char *record;     // buffer of 25 octets + 1 octets of end
	size_t record_len;
	bool   was_pinged;
};

/* Major will always be dynamically allocated */
static int major;
static struct gistre_card_dev *gc_dev;

/*
 * Function use during write command
 */

int len(char *str){		// return the length of a char
  int i = 0;
  while( str[i] != '\0'){
    i++;
  }
  return i+1;
}

char** split( char *str){ 	// split a string with the separator ':'
  
  int i = 0;
  
  char *tmp = kmalloc((25+1) * sizeof(char), GFP_KERNEL);
  int tmp_i = 0;
  
  char **res = kmalloc(3 * sizeof(char*), GFP_KERNEL);
 
  int n = 0;
  while(n < 3)
	  res[n++] = NULL;

  int res_i = 0;
  
  
  
  while(str[i] != '\0' && tmp_i < 25){
  
    if(str[i] != ':'){
      tmp[tmp_i++] = str[i];
    }
    else{
      tmp[tmp_i] = '\0';
      size_t a = len(tmp);
      char *word = kmalloc(a * sizeof(char), GFP_KERNEL);
      strncpy(word,tmp,a);
      tmp_i = 0;
      res[res_i++]=word;
    }  
    i++;
  }
  
  tmp[tmp_i] = '\0';
  size_t a = len(tmp);
  char *word = kmalloc(a * sizeof(char), GFP_KERNEL);
  strncpy(word,tmp,a);
  res[res_i++]=word;
  
  kfree(tmp);
  
  return res;
  
}


/*
 * Character device filed operations
 */

int gistre_card_open(struct inode *inode, struct file *file) {

	unsigned int i_major = imajor(inode);
	unsigned int i_minor = iminor(inode);

	pr_info("%s()\n", __func__);

	if (i_major != major) {
		pr_err("Invalid major %d, expected %d\n", i_major, major);
		return -EINVAL;
	}
	if (i_minor != 0) {
		pr_err("pingpong driver only handles minor 0!\n");
		return -EINVAL;
	}

	/* Make file descriptor "remember" its mirror device object,
	 * for later use in open() and write(). */
	file->private_data = gc_dev;

	return 0;
}

int gistre_card_release(struct inode *inode /* unused */,
        struct file *file /* unused */) {

	pr_info("%s()\n", __func__);

	/* Nothing in particular to do */
	return 0;
}

//-------------------------------------------------------------------------

ssize_t gistre_card_write(struct file *file, const char __user *buf,
        size_t len, loff_t *off /* unused */) {

	// Definition dev & input_buffer
	struct gistre_card_dev *dev;
	char gc_in_buf[MAXIMUM_BUFSIZE+1];

	pr_info("%s", __func__);

	// Récupération dev 
	dev = (struct gistre_card_dev *)file->private_data;
	
	//definition memoire pour input_buffer
	memset(gc_in_buf, 0, MAXIMUM_BUFSIZE+1);
	
	// len : taille de command 'write'
	len = min(len, MAXIMUM_BUFSIZE);


	if (copy_from_user(gc_in_buf, buf, len)) {
		pr_err("Failed to copy user data\n");
		return -EFAULT;
	}

	
	char **splited_command = split(gc_in_buf);
	
	if(strcmp(splited_command[0], "mem_read") == 0){
		pr_info("I was pinged!\n");
		dev->was_pinged = true;
	} 
	else if (strcmp(splited_command[0], "mem_write") == 0){
		dev->record = splited_command[2];
		size_t tmp;

		kstrtoul(splited_command[1],0,&tmp);
		len = min(tmp,GISTRE_CARD_BUFSIZE);
		dev->record_len = len;

		pr_info("The recorded word is '%s' and his length is %lu", dev->record, dev->record_len);
	} else {
		return -EFAULT;
	}
	return len;
}


//-----------------------------------------ICI //TODO// ---------------------------------

ssize_t gistre_card_read(struct file *file, char __user *buf,
        size_t len, loff_t *off /* unused */) {

	struct gistre_card_dev *dev;
	/* Don't forget trailing NUL */
	char gc_out_buf[GISTRE_CARD_BUFSIZE+1];

	pr_info("%s(len=%zu)\n", __func__, len);

	dev = (struct gistre_card_dev *)file->private_data;
	if (dev->was_pinged) {
		/* Answer with a pong. Don't handle offsets */
		len = min(len, GISTRE_CARD_BUFSIZE+1);
		len = min(len, dev->record_len+1);
		memset(gc_out_buf, 0, GISTRE_CARD_BUFSIZE+1);
		memcpy(gc_out_buf, dev->record, GISTRE_CARD_BUFSIZE);
		if (copy_to_user(buf, gc_out_buf, len)) {
			pr_err("Failed to copy data to user\n");
			return -EFAULT;
		}
		dev->was_pinged = false;
	}
	else {
		len = 0;
	}

	return len;
}

/*
 *  Gistre Card fops, creation & destroy
 */

static struct file_operations gistre_card_fops = {
	.owner   = THIS_MODULE,
	.open    = gistre_card_open,
	.release = gistre_card_release,
	.read    = gistre_card_read,
	.write   = gistre_card_write,
	/* Others functions are using the kernel's defaults */
};

static struct gistre_card_dev *gistre_card_create(void) {

	struct gistre_card_dev *dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (! dev) {
		return NULL;
	}

	dev->cdev.owner = THIS_MODULE;
	cdev_init(&dev->cdev, &gistre_card_fops);

	dev->record = NULL;
	dev->record_len = 0;
	dev->was_pinged = false;

	return dev;
}

static void gistre_card_destroy(struct gistre_card_dev *dev) {

	cdev_del(&dev->cdev);
	kfree(dev);
}

/*
 * Init & Exit
 */

__exit
static void gistre_card_exit(void) {

	dev_t dev;

	dev = MKDEV(major, 0);
	gistre_card_destroy(gc_dev);
	unregister_chrdev_region(dev, 1);
}

__init
static int gistre_card_init(void) {

	dev_t dev;
	int ret;
	const char devname[] = "gistre_card";

	/* Allocate major */
	ret = alloc_chrdev_region(&dev, 0, 1, devname);
	if (ret < 0) {
		pr_info("Failed to allocate major\n");
		return 1;
	}
	else {
		major = MAJOR(dev);
		pr_info("Got major %d\n", major);
	}

	/* Register char device */
	gc_dev = gistre_card_create();
	if (! gc_dev) {
		pr_err("Failed to init pingpong_dev\n");
		return -ENOMEM;
	}

	if (cdev_add(&gc_dev->cdev, dev, 1) < 0) {
		pr_err("Failed to register char device\n");
		return -ENOMEM;
	}

	return 0;
}

module_init(gistre_card_init);
module_exit(gistre_card_exit);

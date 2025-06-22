#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ssaz, 2024");
MODULE_DESCRIPTION("Module hello bye bye...");

/* ajout param  */
static int btn;
module_param(btn, int, 0);
MODULE_PARM_DESC(btn, "num du port du bouton");

/* ajout LEDS */
#define NBMAX_LED 32
static int leds[NBMAX_LED];
static int nb_led;
module_param_array(leds, int, &nb_led,0);
MODULE_PARM_DESC(LEDS, "tab des num de port LED");

/* major */
static int major;

static int 
open_led_XY(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "open()\n");
    return 0;
}

static ssize_t 
read_led_XY(struct file *file, char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "read()\n");
    return count;
}

static ssize_t 
write_led_XY(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "write()\n");
    return count;
}

static int 
release_led_XY(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "close()\n");
    return 0;
}

struct file_operations fops_led =
{
    .open       = open_led_XY,
    .read       = read_led_XY,
    .write      = write_led_XY,
    .release    = release_led_XY 
};


static int __init led_module_init(void)
{
    /* ini major */
    major = register_chrdev(0,"led_s",&fops_led);

	int i;
	printk(KERN_DEBUG "hello ssaz :)\n");
	printk(KERN_DEBUG "bouton = %d\n", btn);
	for(i = 0; i < nb_led; i++){
		printk(KERN_DEBUG "LED %d = %d\n", i, leds[i]);
	}
	return 0;
}

static void __exit led_module_cleanup(void)
{
    unregister_chrdev(major,"led_s");
	printk(KERN_DEBUG "bye ssaz :)\n");
}

module_init(led_module_init);
module_exit(led_module_cleanup);

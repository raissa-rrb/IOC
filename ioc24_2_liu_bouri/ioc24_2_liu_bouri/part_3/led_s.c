#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <mach/platform.h>

#define GPIO_LED0   4
#define GPIO_LED1   17
#define GPIO_BP     18

#define GPIO_FSEL_INPUT  0
#define GPIO_FSEL_OUTPUT 1

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

/* etape 4 struct GPIO */
static const int LED0 = 4;

struct gpio_s
{
    uint32_t gpfsel[7];
    uint32_t gpset[3];
    uint32_t gpclr[3];
    uint32_t gplev[3];
    uint32_t gpeds[3];
    uint32_t gpren[3];
    uint32_t gpfen[3];
    uint32_t gphen[3];
    uint32_t gplen[3];
    uint32_t gparen[3];
    uint32_t gpafen[3];
    uint32_t gppud[1];
    uint32_t gppudclk[3];
    uint32_t test[1];
}
volatile *gpio_regs = (struct gpio_s *)__io_address(GPIO_BASE);

/* fonctions GPIO */
static void gpio_fsel(int pin, int fun)
{
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs->gpfsel[reg] = (gpio_regs->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

static void gpio_write(int pin, bool val)
{
    if (val)
        gpio_regs->gpset[pin / 32] = (1 << (pin % 32));
    else
        gpio_regs->gpclr[pin / 32] = (1 << (pin % 32));
}

static int gpio_read(int pin){
    uint32_t reg = pin /32;
    uint32_t reg_value = gpio_regs->gplev[reg]>>pin & 0x1;
    return reg_value;

}

static int 
open_led_XY(struct inode *inode, struct file *file) {

    printk(KERN_DEBUG "open()\n");
    gpio_fsel(GPIO_BP,GPIO_FSEL_INPUT);
    gpio_fsel(GPIO_LED0,GPIO_FSEL_OUTPUT);
    gpio_fsel(GPIO_LED1,GPIO_FSEL_OUTPUT);
    return 0;
}

static ssize_t 
read_led_XY(struct file *file, char *buf, size_t count, loff_t *ppos) {
    //printk(KERN_DEBUG "read()\n");
    uint32_t bp = gpio_read(GPIO_BP);
    if (bp == 1){
        printk(KERN_DEBUG "BP on\n");
        *buf = "1";
    }
    else{
        printk(KERN_DEBUG "BP off\n");
        *buf = "0";
    }
    return count;
}

static ssize_t 
write_led_XY(struct file *file, const char *buf, size_t count, loff_t *ppos) {

    if(*buf == '1'){
        gpio_write(GPIO_LED0,1);
        gpio_write(GPIO_LED1,1);
    }
    else{
        gpio_write(GPIO_LED0,0);
        gpio_write(GPIO_LED1,0);
    }
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
    i = 0;
	printk(KERN_DEBUG "hello ssaz :)\n");
	printk(KERN_DEBUG "bouton = %d\n", btn);
	for(i ; i < nb_led; i++){
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

#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ssaz, 2024");
MODULE_DESCRIPTION("Module hello bye bye...");

/* ajout param étape 2 */
static int btn;
module_param(btn, int, 0);
MODULE_PARM_DESC(btn, "num du port du bouton");

/* ajout LEDS */
#define NBMAX_LED 32
static int leds[NBMAX_LED];
static int nb_led;
module_param_array(leds, int, &nb_led,0);
MODULE_PARM_DESC(LEDS, "tab des num de port LED");

static int __init hello_module_init(void)
{
	int i;
	printk(KERN_DEBUG "hello ssaz :)\n");
	printk(KERN_DEBUG "bouton = %d\n", btn);
	for(i = 0; i < nb_led; i++){
		printk(KERN_DEBUG "LED %d = %d\n", i, leds[i]);
	}
	return 0;
}

static void __exit hello_module_cleanup(void)
{
	printk(KERN_DEBUG "bye ssaz :)\n");
}

module_init(hello_module_init);
module_exit(hello_module_cleanup);

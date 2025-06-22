#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <asm/io.h>
#include <mach/platform.h>
#include <asm/delay.h>
#include "lcd_lb.h"


/*******************************************************************************
 * GPIO Pins
 ******************************************************************************/
#define RS 7
#define E  27
#define D4 22
#define D5 23
#define D6 24
#define D7 25

#define GPIO_FSEL_INPUT  0
#define GPIO_FSEL_OUTPUT 1

#define GPIO_BP 18

#define RPI_BLOCK_SIZE  0xB4
#define RPI_GPIO_BASE   0x20200000

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raissa Owen, 2024");
MODULE_DESCRIPTION("Module lcd");

// /* ajout param  */
// static int btn;
// module_param(btn, int, 0);
// MODULE_PARM_DESC(btn, "num du port du bouton");

// /* ajout LEDS */
// #define NBMAX_LED 32
// static int leds[NBMAX_LED];
// static int nb_led;
// module_param_array(leds, int, &nb_led,0);
// MODULE_PARM_DESC(LEDS, "tab des num de port LED");

/* major */
static int major;

/* etape 4 struct GPIO */
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

void gpio_write(int gpio, int value)
{
    int regnum = gpio / 32;
    int offset = gpio % 32;
    if (value&1)
        gpio_regs->gpset[regnum] = (0x1 << offset);
    else
        gpio_regs->gpclr[regnum] = (0x1 << offset);
}

static int gpio_read(int pin){
    uint32_t reg = pin /32;
    uint32_t reg_value = gpio_regs->gplev[reg]>>pin & 0x1;
    return reg_value;
}

/*******************************************************************************
 * LCD's Instructions ( source = doc )
 * Ces constantes sont utilisées pour former les mots de commandes
 * par exemple dans la fonction lcd_init()
 ******************************************************************************/

/* commands */
#define LCD_CLEARDISPLAY        0b00000001
#define LCD_RETURNHOME          0b00000010
#define LCD_ENTRYMODESET        0b00000100
#define LCD_DISPLAYCONTROL      0b00001000
#define LCD_CURSORSHIFT         0b00010000
#define LCD_FUNCTIONSET         0b00100000
#define LCD_SETCGRAMADDR        0b01000000
#define LCD_SETDDRAMADDR        0b10000000

/* flags for display entry mode : combine with LCD_ENTRYMODESET */
#define LCD_EM_RIGHT            0b00000000
#define LCD_EM_LEFT             0b00000010
#define LCD_EM_DISPLAYSHIFT     0b00000001
#define LCD_EM_DISPLAYNOSHIFT   0b00000000

/* flags for display on/off control : combine with LCD_DISPLAYCONTROL */
#define LCD_DC_DISPLAYON        0b00000100
#define LCD_DC_DISPLAYOFF       0b00000000
#define LCD_DC_CURSORON         0b00000010
#define LCD_DC_CURSOROFF        0b00000000
#define LCD_DC_BLINKON          0b00000001
#define LCD_DC_BLINKOFF         0b00000000

/* flags for display/cursor shift : combine with LCD_CURSORSHIFT */
#define LCD_CS_DISPLAYMOVE      0b00001000
#define LCD_CS_CURSORMOVE       0b00000000
#define LCD_CS_MOVERIGHT        0b00000100
#define LCD_CS_MOVELEFT         0b00000000

/* flags for function set : combine with LCD_FUNCTIONSET */
#define LCD_FS_8BITMODE         0b00010000
#define LCD_FS_4BITMODE         0b00000000
#define LCD_FS_2LINE            0b00001000
#define LCD_FS_1LINE            0b00000000
#define LCD_FS_5x10DOTS         0b00000100
#define LCD_FS_5x8DOTS          0b00000000

/*LCD function*/
void lcd_strobe(void)
{
    gpio_write(E, 1);
    udelay(1);
    gpio_write(E, 0);
}

void lcd_write4bits(int data)
{
    /* first 4 bits */
    gpio_write(D7, data>>7);
    gpio_write(D6, data>>6);
    gpio_write(D5, data>>5);
    gpio_write(D4, data>>4);
    lcd_strobe();

    /* second 4 bits */
    gpio_write(D7, data>>3);
    gpio_write(D6, data>>2);
    gpio_write(D5, data>>1);
    gpio_write(D4, data>>0);
    lcd_strobe();
}

void lcd_command(int cmd)
{
    gpio_write(RS, 0);
    lcd_write4bits(cmd);
    udelay(2000);               // certaines commandes sont lentes 
}

void lcd_data(int character)
{
    gpio_write(RS, 1);
    lcd_write4bits(character);
    udelay(50);
}

void lcd_init(void)
{
    gpio_write(E, 0);
    lcd_command(0b00110011);    /* initialization */
    lcd_command(0b00110010);    /* initialization */
    lcd_command(LCD_FUNCTIONSET | LCD_FS_4BITMODE | LCD_FS_2LINE | LCD_FS_5x8DOTS);
    lcd_command(LCD_DISPLAYCONTROL | LCD_DC_DISPLAYON | LCD_DC_CURSOROFF);
    lcd_command(LCD_ENTRYMODESET | LCD_EM_RIGHT | LCD_EM_DISPLAYNOSHIFT);
}

void lcd_clear(void)
{
    lcd_command(LCD_CLEARDISPLAY);
    lcd_command(LCD_RETURNHOME);
}

int ligne =0;
int col=0;

void set_cursor(int x, int y){

    int a[] = { 0, 0x40, 0x14, 0x54 }; //permet de choisir la ligne =>y
    int i;
    ligne = y;
    col = x;
    lcd_command(LCD_SETDDRAMADDR + a[ligne]); //on est sur la bonne ligne
    /* on shift à droite ujusqu'à avoir la bonne colonne */
    for(i = 0; i < x ; i++){
        lcd_command(LCD_CURSORSHIFT | LCD_CS_MOVERIGHT);
    }
}

void lcd_message(const char *txt)
{
    int a[] = { 0, 0x40, 0x14, 0x54 };
    int len = 20;
    int i,l,j;
    int space = len-col;
    for (i = 0, l = 0; (l < 4) && (i < strlen(txt)); l++){
        if(l>0){
            lcd_command(LCD_SETDDRAMADDR + a[ligne+1]);
            ligne++;
            space = 20;
        }
        for (j=0; (j<space) && i<strlen(txt); j++,i++){
            //printk(KERN_DEBUG "lcd_message() texte[%d] : %d\n",i, txt[i]);
            //printk("j=%d\n",j);
            if(txt[i] == '\n'){
                lcd_command(LCD_SETDDRAMADDR + a[l+1]);
                //printk("test if %c\n",txt[i]);
            }
            else{
                lcd_data(txt[i]);
                //printk("elsetest  %c\n",txt[i]);
            }

        }
    }
}

/*appel système*/

static int 
open_lcd_lb(struct inode *inode, struct file *file) {

    printk(KERN_DEBUG "open()\n");
    gpio_fsel(RS,GPIO_FSEL_OUTPUT);
    gpio_fsel(E ,GPIO_FSEL_OUTPUT);
    gpio_fsel(D4,GPIO_FSEL_OUTPUT);
    gpio_fsel(D5,GPIO_FSEL_OUTPUT);
    gpio_fsel(D6,GPIO_FSEL_OUTPUT);
    gpio_fsel(D7,GPIO_FSEL_OUTPUT);

    lcd_init();
    lcd_clear();
    
    return 0;
}

static ssize_t 
read_lcd_lb(struct file *file, char *buf, size_t count, loff_t *ppos) {
    //printk(KERN_DEBUG "read()\n");
    uint32_t bp = gpio_read(GPIO_BP);
    if (bp == 1){
        printk(KERN_DEBUG "BP on\n");
        buf = "1";
    }
    else{
        printk(KERN_DEBUG "BP off\n");
        buf = "0";
    }
    return count;
}

static ssize_t 
write_lcd_lb(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    char* texte = (char*)kmalloc(count, GFP_KERNEL);
    if(NULL == texte){
        return -ENOMEM;
    }

    ligne = 0;
    col   = 0;

    if(copy_from_user(texte, buf, count)) return -ENOMEM;
    printk(KERN_DEBUG "write() texte : %s\n", texte);
    lcd_message(texte);
    kfree(texte);
    return count;
}

static int 
release_lcd_lb(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "close()\n");
    return 0;
}

static int
LCDIOCT_FUNC(struct file* file, unsigned int cmd, unsigned long arg){
    struct cord_xy cord;

    if(_IOC_TYPE(cmd) != IOC_MAGIC)
        return -EINVAL;
    switch(cmd){
        case LCDIOCT_CLEAR:
            file->f_pos=0;
            lcd_clear();
            break;
        case LCDIOCT_SETXY:
            if(copy_from_user(&cord, (void*)arg, _IOC_SIZE(cmd)) != 0)
                return -EINVAL;
            // ligne = cord.line;
            // col =cord.row;
            set_cursor(cord.row, cord.line);
            printk("line : %d\n");
            printk("col : %d");
            break;
        default : return -EINVAL;

    }

    return 0;
}

struct file_operations fops_lcd =
{
    .open       = open_lcd_lb,
    .read       = read_lcd_lb,
    .write      = write_lcd_lb,
    .unlocked_ioctl = LCDIOCT_FUNC,
    .release    = release_lcd_lb
};


static int __init lcd_module_init(void)
{
    /* ini major */
    major = register_chrdev(0,"lcd_lb",&fops_lcd);
	printk(KERN_DEBUG "Module LCD init\n");
	return 0;
}

static void __exit lcd_module_cleanup(void)
{
    unregister_chrdev(major,"lcd_lb");
	printk(KERN_DEBUG "Module LCD ciao !\n");
}

module_init(lcd_module_init);
module_exit(lcd_module_cleanup);

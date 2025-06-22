//------------------------------------------------------------------------------
// Headers that are required for printf and mmap
//------------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pthread.h> 

//------------------------------------------------------------------------------
// GPIO ACCES
//------------------------------------------------------------------------------

#define BCM2835_PERIPH_BASE     0x20000000
#define BCM2835_GPIO_BASE       ( BCM2835_PERIPH_BASE + 0x200000 )

#define GPIO_LED0   4
#define GPIO_LED1   17
#define GPIO_BP     18

#define GPIO_FSEL_INPUT  0
#define GPIO_FSEL_OUTPUT 1

int BP_ON = 0;
int BP_OFF = 0;

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
};

struct gpio_s *gpio_regs_virt; 


static void 
gpio_fsel(uint32_t pin, uint32_t fun)
{
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs_virt->gpfsel[reg] = (gpio_regs_virt->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

static void 
gpio_write (uint32_t pin, uint32_t val)
{
    uint32_t reg = pin / 32;
    uint32_t bit = pin % 32;
    if (val == 1) 
        gpio_regs_virt->gpset[reg] = (1 << bit);
    else
        gpio_regs_virt->gpclr[reg] = (1 << bit);
}

static void gpio_read(uint32_t pin){
    uint32_t reg = pin /32;
    uint32_t reg_value = gpio_regs_virt->gplev[reg]>>pin & 0x1;
    if(reg_value){
        BP_ON = 0;
        BP_OFF = 1;
    }
    else{
        BP_ON = 1;
        BP_OFF = 0;
    }
}

//------------------------------------------------------------------------------
// Access to memory-mapped I/O
//------------------------------------------------------------------------------

#define RPI_PAGE_SIZE           4096
#define RPI_BLOCK_SIZE          4096

static int mmap_fd;

static int
gpio_mmap ( void ** ptr )
{
    void * mmap_result;

    mmap_fd = open ( "/dev/mem", O_RDWR | O_SYNC );

    if ( mmap_fd < 0 ) {
        return -1;
    }

    mmap_result = mmap (
        NULL
      , RPI_BLOCK_SIZE
      , PROT_READ | PROT_WRITE
      , MAP_SHARED
      , mmap_fd
      , BCM2835_GPIO_BASE );

    if ( mmap_result == MAP_FAILED ) {
        close ( mmap_fd );
        return -1;
    }

    *ptr = mmap_result;

    return 0;
}

void
gpio_munmap ( void * ptr )
{
    munmap ( ptr, RPI_BLOCK_SIZE );
}

//------------------------------------------------------------------------------
// Main Programm
//------------------------------------------------------------------------------

void
delay ( unsigned int milisec )
{
    struct timespec ts, dummy;
    ts.tv_sec  = ( time_t ) milisec / 1000;
    ts.tv_nsec = ( long ) ( milisec % 1000 ) * 1000000;
    nanosleep ( &ts, &dummy );
}

struct args{
    int pin;
    int period;
};

void * blink(void *data) {

    int half_period;
    struct args *args = data;

    half_period = args->period/2;
    uint32_t volatile * gpio_base = 0;
   
    // map GPIO registers
    // ---------------------------------------------

    if ( gpio_mmap ( (void **)&gpio_regs_virt ) < 0 ) {
        printf ( "-- error: cannot setup mapped GPIO.\n" );
        exit ( 1 );
    }

    // Setup GPIO of LED_pin to output
    // ---------------------------------------------
    
    gpio_fsel(args->pin, GPIO_FSEL_OUTPUT);
 
    uint32_t val = 0;

    printf ( "-- info: start blinking.\n" );

    while (1) {
        gpio_write ( args->pin, val );
        delay ( half_period );
        val = 1 - val;
    }
}

void * write_bp(void *data) {

    uint32_t volatile * gpio_base = 0;
    struct args *args = data;
   
    // map GPIO registers
    // ---------------------------------------------

    if ( gpio_mmap ( (void **)&gpio_regs_virt ) < 0 ) {
        printf ( "-- error: cannot setup mapped GPIO.\n" );
        exit ( 1 );
    }

    // Setup GPIO of LED0 to output
    // ---------------------------------------------
    
    gpio_fsel(args->pin, GPIO_FSEL_OUTPUT);

    uint32_t val = 0;
    uint32_t temp = 1;

    printf ( "-- info: start switching.\n" );

    while (1) {
        if(BP_ON & temp){
            temp = 0;
            val = !val;
            gpio_write ( args->pin, val);
            (val ? printf("ON\n") : printf("OFF\n"));
            delay(200);
        }
        if(BP_OFF) temp = 1;
    }
}

void * read_bp() {

    uint32_t volatile * gpio_base = 0;
   
    // map GPIO registers
    // ---------------------------------------------

    if ( gpio_mmap ( (void **)&gpio_regs_virt ) < 0 ) {
        printf ( "-- error: cannot setup mapped GPIO.\n" );
        exit ( 1 );
    }

    // Setup GPIO_BP of BP to intput
    // ---------------------------------------------
    
    gpio_fsel(GPIO_BP, GPIO_FSEL_INPUT);

    printf ( "-- info: start reading\n" );

    while (1) {
        gpio_read(GPIO_BP);
        delay(20);
    }
}

int
main ( int argc, char **argv )
{

    int period1 =  1000; //valeur par défaut 1 Hz

    if ( argc > 1 ) {
        period1 = atoi ( argv[1] );
    }

    struct args *arg0 = malloc(sizeof(struct args));
    struct args *arg1 = malloc(sizeof(struct args));
    arg0->period = -1; //ne prend pas de période
    arg0->pin    = GPIO_LED0;
    arg1->period = period1;
    arg1->pin    = GPIO_LED1;

    //création du thread
    pthread_t thread_blink1; 
    pthread_t thread_write;
    pthread_t thread_read;

    //pthread_create(&thread_blink0, NULL, blink, arg0);
    pthread_create(&thread_blink1, NULL, blink   , arg1);
    pthread_create(&thread_write , NULL, write_bp, arg0);
    pthread_create(&thread_read  , NULL, read_bp , NULL);
    
    pthread_join(thread_blink1, NULL);
    pthread_join(thread_write , NULL);
    pthread_join(thread_read  , NULL);

    return 0;
}

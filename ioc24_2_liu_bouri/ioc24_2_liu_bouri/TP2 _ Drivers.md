# TP2 : Les Drivers !

## Auteurs

- LIU Owen 28726287
- BOURI Raïssa 28602820

## Création et test d'un module noyau

### Ecriture de **module.c**
- Quelle fonction est exécutée lorsqu'on insère le module du noyau ?
> Lors de l'insertion du module dans le noyau c'est la fonction **hello_init** qui est éxecutée. 
> C'est avec la commande insmod que l'on insère le module. Avec la commande **dmesg** nous pouvons voir l'écriture faite avec **hello_init** : Hello World ssaz!
-  Quelle fonction est exécutée lorsqu'on enlève le module du noyau ? 
> Lorsque l'on retire le module du noyau c'est la fonction **hello_cleanup** qui est executée.
> Pour la délétion de module c'est la commande **rmmod** qu'il faut utiliser. 
> De la même manière, avec la commande **dmesg** nous pouvons voir le message "Ciaoooo!" s'afficher. 

### Compilation de module.c

> Remarque :-1: : Le Makefile fournit possède une erreur de path : 

    CROSSDIR        = /users/enseig/franck/!!!E_IOC!!!
    CROSSDIR        = /users/enseig/franck/E-IOC

> Sur notre PC les fichiers du noyau Raspberry étaient déjà présent.

### Code du module "Hello World"
    #include <linux/module.h>
    #include <linux/init.h>

    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Bouri Liu, 2024");
    MODULE_DESCRIPTION("Module test, ouiiiiiIIIII");

    static int __init hello_init(void){
        printk(KERN_DEBUG "Hello World ssaz!\n");
        return 0;
    }

    static void __exit hello_cleanup(void){
        printk(KERN_DEBUG "Ciaoooo!\n");
    }

    module_init(hello_init);
    module_exit(hello_cleanup);
    
## Ajout de paramètres au module

> Remarque :+1:  : il est possible d'ajouter un ou plusieurs paramètre avec soit :
     
    module_param(nom du param, type du param, valeur par défaut) 
    module_param_array(nom de l'array, types de données contenues, adresse de la variale contenant l'array, valeur par défaut)

> Nous avons découvert que si l'on déclare un seul paramètre puis, que nous en ajoutons un autre ou un array, les paramètres s'ajoutent. 
> Il n'y a pas d'écrasement des paramètres.

    #include <linux/module.h>
    #include <linux/init.h>
    #define NBMAX_LED 32

    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Bouri Liu, 2024");
    MODULE_DESCRIPTION("Module test, ouiiiiiIIIII");

    static int btn;
    module_param(btn, int, 0);
    MODULE_PARM_DESC(btn, "Port du bouton :");

    static int leds[NBMAX_LED];
    static int nbled;
    module_param_array(leds, int, &nbled, 0);
    MODULE_PARM_DESC(LEDS, "tableau des numéros de port LED");

    static int __init hello_init(void){
        int i;
        printk(KERN_DEBUG "Hello World ssaz!\n");
        printk(KERN_DEBUG "btn=%d \n", btn);
        for(i=0; i < nbled; i++)
            printk(KERN_DEBUG "LED_%d = %d\n", i, leds[i]);
        return 0;
    }

    static void __exit hello_cleanup(void){
        printk(KERN_DEBUG "Ciaoooo!\n");
    }

    module_init(hello_init);
    module_exit(hello_cleanup);
    

- Pour vérifier que les paramètres ont bien été lu on éxécute la commande **dmesg** qui nous donne : 

```
[ 2471.576647] Hello World ssaz!
[ 2471.576687] btn=18 
[ 2471.576699] LED_0 = 4
[ 2471.576712] LED_1 = 17
```

## Création d'un driver qui ne fait rien...à nos yeux
Avec les instructions du TME: 
- Pour créer ce module nous avons d'abord copié les appels systèmes avant les fonctions d'initialisation et de cleanup. 
- Ensuite, nous avons déclaré une nouvelle variable major et dans la fonction d'iniitalisation du module nous lui avons affecté sa valeur, ici nous laissons linux choisir sa valeur.
- Dans la fonction de cleanup du module,nous libérons cette valeur de major. 
- Dans le fichier proc/devices nous pouvons constater la création de notre driver led_BL qui a pour valeur de major : 246.  
```

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#define NBMAX_LED 32

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bouri Liu, 2024");
MODULE_DESCRIPTION("Module test, ouiiiiiIIIII");

/*Parameter button*/
static int btn;
module_param(btn, int, 0);
MODULE_PARM_DESC(btn, "Port du bouton :");

/*Parameter LEDS*/
static int leds[NBMAX_LED];
static int nbled;
module_param_array(leds, int, &nbled, 0);
MODULE_PARM_DESC(LEDS, "tableau des numéros de port LED");

/*Parameter for driver*/
static int major;

/*Appel Système*/
static int
open_led_BL(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "open()\n");
    return 0;
}

static ssize_t 
read_led_BL(struct file *file, char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "read()\n");
    return count;
}

static ssize_t 
write_led_BL(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "write()\n");
    return count;
}

static int 
release_led_BL(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "close()\n");
    return 0;
}

struct file_operations fops_led =
{
    .open       = open_led_BL,
    .read       = read_led_BL,
    .write      = write_led_BL,
    .release    = release_led_BL 
};

/*Init module*/
static int __init led_BL_init(void){
	int i;
	major = register_chrdev(0, "led_BL", &fops_led); // 0 est le numéro majeur qu'on laisse choisir par linux
	printk(KERN_DEBUG "Hello World ssaz!\n");
	printk(KERN_DEBUG "btn=%d \n", btn);
	for(i=0; i < nbled; i++) printk(KERN_DEBUG "LED_%d = %d\n", i, leds[i]);
	return 0;
}

/*Exit module*/
static void __exit led_BL_cleanup(void){
	unregister_chrdev(major, "led_BL");
	printk(KERN_DEBUG "Ciaoooo!\n");
}

module_init(led_BL_init);
module_exit(led_BL_cleanup);
```

Une chose importante à noter, et le "lien" fait entre les appels systèmes du noyau, tels que **open()**, **read()** et **write()**, avec les fonctions que nous avons codées : **open_led_BL()** , **read_led_BL()** et **write_led_BL()**.
Ce lien est fait dans la structure **file_operations**. 
Nous pouvons voir que l'on assigne à **open()** la fonction **open_led_BL()**.

## Suite à echo
- La commande echo permet d'afficher sur le terminal la chaine de charactère passée en paramètre : 
```
[bouri@gallus lab2]$ echo "hello ssaz"
hello ssaz
```
- Ce symbole **'>'** redirige la sortie de la commande **echo** : la chaine de charactère passée en argument,  vers notre device led_BL
- Celui-ci a donc reçu en paramètre : "rien".
- La commande **dd bs=1 count=1 < /dev/led_BL** lit un octet de de /dev/led_BL

```
[ 5601.466490] Hello World ssaz!
[ 5601.466528] btn=18 
[ 5601.466542] LED_0 = 4
[ 5601.466556] LED_1 = 17
[ 5670.516778] open()
[ 5670.516924] write()
[ 5670.516975] close()
[ 5691.853102] open()
[ 5691.865402] read()
```
Notre device **open** le fichier puis **write** dedans car c'est ce que la commande **echo** lui demande, puis il se ferme. 
Avec la commande **dd bs=1 count=1 < /dev/led_BL** nous voulons lire dans le fichier et c'est bien un **open()** suivi d'un **read()** que nous obtenons. 
Ces tests nous montrent bien le lien fait entre les appels systèmes du noyau et les fonctions de notre module.

## Avec l'automatisation du chargement et effacement du driver 
```
pi@raspberrypi ~/liu_bouri/lab2 $ sudo ./insdev.sh led_BL led=2
=> Device /dev/led_BL created with major=244
pi@raspberrypi ~/liu_bouri/lab2 $ sudo ./rmdev.sh led_BL LED=2
=> Device /dev/led_BL removed
```
Nous pouvons remarquer que insdev récupère le numéro major de leb_BL. 
C'est cette ligne du insdev.sh qui permet sa récupèration : 
`major=$(awk "\$2==\"$module\" {print \$1;exit}" /proc/devices)`
- La commande **awk** permet d'extraire une information.

## Accès au GPIO depuis les fonctions du pilote
- **Remarque importante pour nos prochains TP**:
    - Il semble que le lien fait dans la structure **file_operation** ne "remplace" pas les appels systèmes du noyau mais les enrichi. 
    - Dans notre **open_led_BL()** nous ne renvoyons pas le file descriptor du fichier cepandant il est quand même récupéré dans le main. 
    - Une erreur que nous avons faite et de considér que les arguments donnés à **open()** s'écrasaient, notamment les droits d'ouverture. 
    - Cela à posé problème car nous l'avions ouvert en lecture seule. 
    - À ne pas refaire !


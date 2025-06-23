# TP3 : Pilote d'un écran LCD 
## 1. Configuration du GPIO pour le pilotage de l'écran 

Données de l'énoncé : 
- Signaux de contrôle : 
    - RS : 1 pour envoi d'une donnée ou 0 pour l'envoi d'une commande 
    - E : signal de validation, la valeur sur le bus de donnée de 4bits est prise en compte sur front descendant de ce signal. 

Et le mapping donné : 
|   Signal LCD   |      GPIO      |
|:--------------:|:--------------:|
|       RS       |       7        |
|       E        |       27       |
| D4, D5, D6, D7 | 22, 23, 24, 25 |


À l'aide de la documentation de l'afficheur : 
- Comment faut-il configurer les GPIOs pour les différents signaux de l'afficheur LCD ? 
    > Il faut configurer les GPIO mappé à RS et D4, D5, D6 et D7 en sortie. 
- Comment écrire les valeurs vers le LCD ?
    > Il faut mettre RS à 1.
- Quelles valeurs doivent être envoyées vers l'afficheur pour réaliser l'initialisation? 
    > Pour l'initialisation plusieurs instructions: 
        >- Display clear
        >- Assigner les valeurs DL, N et F grâce à function set 
        >- Contrôle de l'écran 
            - D = 0 : écran éteint
            - C = 0 : curseur désactivé
            - B = 0 : clignotement du curseur désactivé
        >- Sélectionner le mode d'entrée : 
            - Sélectionne la direction du curseur 
            - Sans défilement de l'écran 
- Comment demander l'affichage d'un caractère ? 
    > Il faut lire les donées stockées dans la DDRAM ou la CGRAM avec l'instruction **read data**
- Comment envoyer des commandes telles que : l'effacement de l'écran, le déplacement du curseur, ... ?
    > Il faut mettre RS à 0 : envoi d'une commande et il faut également assigner les ports DBs à leur bonne valeur en fonction de la commande voulue **( mettre un example)**

## 2. Fonctionnement de l'écran et fonctions de base
  
 - À quoi sert le mot clé volatile ? 
     > Il permet d'empêcher le compilateur d'optimiser ces variables. 
     > Dans notre cas, les valeurs des champs de gpio_s  peuvent se modifer sans intervention du programme.
- Expliquer la présence des drapeaux dans open() et mmap()
    >- Dans open() : 
        - O_RDWR : signifie que l'on ouvre le fichier /dev/mem en lecture et écriture. 
        - O_SYNC : permet de guarantir qu'en cas d'écriture le fichier sera mis à jour immédiatement sur le disque. Cela permet de garder la cohérence des données.
    >- mmap() :
        -PROT_REAd et PROT_WRITE: comme pour open, ces deux flags nous donnent les droits d'accés, en lecture et en écriture. 
- Pourquoi appeler munmap()?
    >- Comme lors d'un malloc il faut libéréer la mémoire allouée avec un free, munmap est l'équivalent de free pour mmap.
- Expliquer le rôle des masques tels que LCD_FUNCTIONSET, LCD_FS_4BITMODE, ... : 
    > - Ces masques sont des macro et ils correspondent aux valeurs à donner aux signaux DBs pour avoir la bonne instruction.
    > Par exemple, le masque LCD_CLEARDISPLAY vaut 0b00000001 et il correspond à DB0 = '1' et lesd autres à '0' comme spécifié dans la datasheet. 
- Expliquer le fonctionnement de la fonction **lcd_message()** : 
    > Le tableau a[] contient les offsets pour la DDRAM, nous avons 4 lignes donc bien 4 offsets.
    > Pour chaque caractère, nous l'écrivons sur une ligne et si jamais nous arrivons à la fin de la ligne alors nous incrémentons **l** et passons à la ligne suivante. 


```
void lcd_message(const char *txt)
{

    int a[] = { 0, 0x40, 0x14, 0x54 };
    int len = 20;
    int i, l;

    
    for (i = 0, l = 0; (l < 4) && (i < strlen(txt)); l++) {
        lcd_command(LCD_SETDDRAMADDR + a[l]);
        for (; (i < (l + 1) * len) && (i < strlen(txt)); i++) {
            lcd_data(txt[i]);
        }
    }
}
```
- **a** contient les @ des 4 lignes du LCD
- **len** : nb de caractères max par ligne
- **i** : nb caractères écrit
- tant que **l<4** et que **i< longueur de la chaine** 
     - alors on écrit à la ligne **a[l]**
    - tant que **i < (l+1)x len** => nb car écrit < nb car. max sur tout l'afficheur
    et **i < longueur de la chaine**
        - on écrit le caractère sur le LCD
    - **i++**
- **l++**

> **On sort de la deuxième boucle si on a écrit 20 (ligne 0), 40 (ligne 1), 60 (ligne 3) ou 80 (ligne4) caractères et on incrémente l une fois qu'on en est sortis => on passe à la ligne suivante**
> **On sort de la première boucle, et de la fonction, si l >4 ( on a 4 lignes max ) ou une fois qu'on a écrit tout les caracères de la chaine**

#### Set cursor
- On a une instruction pour le curseur du LCD : la colonne
```
#define LCD_CURSORSHIFT         0b00010000
/* flags for display/cursor shift : combine with LCD_CURSORSHIFT */
#define LCD_CS_DISPLAYMOVE      0b00001000
#define LCD_CS_CURSORMOVE       0b00000000
#define LCD_CS_MOVERIGHT        0b00000100
#define LCD_CS_MOVELEFT         0b00000000
```
- Et pour la ligne on utilise **LCD_SETDDRAMADDR**
```
void lcd_set_cursor(int x, int y)
{
    /* 
    * x = colonne
    * y = ligne
    */
   int i;
   int a[] = { 0, 0x40, 0x14, 0x54 };

   /* ligne */
   lcd_command(LCD_SETDDRAMADDR + a[y]);
   
   /* colonne */
   for ( i = 0; i < x ; i++){
        lcd_command(LCD_CURSORSHIFT | LCD_CS_MOVERIGHT); 
   }

}
```
:hand:  Il faut modifer la ligne **puis** la colonne. 

**ok ça fonctionne pour des chaines qui tiennent sur une seule ligne sinon dès que ça dépasse on saut une ligne ( comportement par défaut ) mais on aimerait bien rester à la ligne d'en dessous**
```
void lcd_message(const char *txt)
{

    int a[] = { 0, 0x40, 0x14, 0x54 };
    int len = 20;
    int i, l, j;

    
    for (i = 0, l = 0; (l < 4) && (i < strlen(txt)); l++) {
        //lcd_command(LCD_SETDDRAMADDR + a[ligne]);
        if(l>0){
            ligne++;
            colonne = 0;
            lcd_command(LCD_SETDDRAMADDR + a[ligne]);
        }
        //for (; (i < (l + 1) * len) && (i < strlen(txt)); i++) {
        for(j =0; (j < (len - colonne)) && (i < strlen(txt)); i++,j++){
            lcd_data(txt[i]);
        }
    }
}
```
maintenant c'est réglé
    

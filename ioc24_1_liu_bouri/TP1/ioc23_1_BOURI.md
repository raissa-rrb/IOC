# TP1 : Hello world! LED et BP

## Auteurs

- BOURI Raïssa
- LIU Owen


## Configuration SSH
>Données de l'énoncé : 
> - Les cartes RaspeberryPi sont connectées au routeur Peri
> - Ce routeur attribue à chaque carte une adresse IP fixe de format 192.168.1.numDePort
> - Ainsi, tout les paquets entrants sur le port du routeur Peri correspondant à une carte (622NumdePort) seront redirigés vers la carte.
> Par exemple, nous nous connectons au port 62223, car nous avons la carte 23, son adresse IP sera  192.168.1.23

- 1. Pourquoi passer par la redirection des ports? 
    >- SSH nous permet d'avoir une connection virtuelle, ici nous nous connectons virtuellement au routeur Peri de manière sécurisée.
    >- L'intérêt de la redirection des ports est de connecter plusieurs machines à un même réseau.
    >-  Dans notre cas, toute les cartes RaspberryPi sont connectées au routeur Peri, et elles reçoivent les données envoyées sur leur port. D'où l'intérêt de nous connecter au routeur Peri. 
- 2. Pourquoi faut-il que vos fichiers soient dans un répertoire propre sur une RaspberryPi ? 
    >- Sur les cartes RaspberryPi il n'y a qu'un seul compte utilisateur, étant donné que nous partageons les cartes entre plusieurs binomes il est nécessaire de créer notre propre répertoire.

## Clignotement d'une LED 
- Nos remarques 
    > Ne pas oublier le sudo avant de lancer l'éxecutable !

- 1. Pourquoi pourrait-il être dangereux de se tromper de broche pour la configuration? 
    > Nous pourrions envoyer de mauvaises informations à des broches non connectées voir pire, à des broches connectées en lecture, et endommager les composants connectés voir la carte. 

- 2. A quoi correspond l'adresse BCM2835_GPIO_BASE ?
    >Cette adresse, stockée dans une Macro, représente la première adresse physique des registres GPIO. Elle permet d'utiliser les ports GPIO plus simplement en facilitant les calculs d'adresses.

- 3. Que représente la structure struct gpio_s ?
    > C'est une structure contenant la carte des registres. Elle permet d'accéder plus facilement aux fonctions des ports (lecture/écriture...).

- 4. Dans quel espace d'adressage est l'adresse **gpio_regs_virt** ?
    > Cette adresse est dans l'espace d'adressage virtuel, d'où son nom : gpio_regs_virt
    
- 5. Dans la fonction **gpio_fsel()**, que contient la variable reg ?
    > Elle contient le numéro du pin divisé par 10, de cette manière nous obtenons la région de la pin.
    > Par exemple, prenons la pin 2, reg = 2/10 = 0. Donc cela correspond à la région 0 (voir diapo 8 cours 1)

- 6. Dans la fonction **gpio_write()**, pourquoi écrire à deux adresses différentes en fonction de la valeur val ?
    > Ici, ces adresses représentent des fonctions différentes : set et clear. Si val vaut 1 on choisit de mettre à 1 le pin, sinon on le met à 0.

- 7. Dans la fonction **gpio_mmap()**, à quoi correspondent les flags de **open()** ?
    > Les flags de open() nous permettent de choisir le mode d'accès : lecture seule (**O_RDONLY**), écriture seule (**O_WRONLY**), ou lecture et écriture (**O_RDWR**). 
    > Le flag **O_SYNC** quant à lui, permet d'effectuer une écriture synchronisée, cela signifie que lors d'un appel à write() le processus courant sera bloqué jusqu'à écriture de la donnée sur le disque pour garantir la cohérence des données.

- 8. Dans la fonction **gpio_mmap()**, commentez les arguments de **mmap()**.
    > Dans l'odre
    > - **void * addr** : adresse à laquelle créer la projection, ici elle vaut NULL donc c'est le noyau qui décide de l'adresse.
    > - **length** : taille de l'objet que l'on veut mapper, ici on donne la taille d'un block.
    > - **prot** : protection mémoire, comme les flags O_RDONLY ou O_WRITE ils définissent le type d'accès voulu.
    > - **flags** : permet de partager ou pas cette projection entre processus. Ici, MAP_SHARED donc c'est partageable entre processus.
    > - **fd** : file descriptor, référence le fichier ouvert
    > - **offset** : position dans le fichier

- 9. Que fait la fonction **delay()** ?
    > Permet de faire dormir le processus pendant une durée déterminée en milliseconde passée en paramètre.
    
- 10. Pourquoi doit-on utiliser sudo ? 
    > Nous voulons accéder à dev/mem qui nous donne l'accès à la mémoire physique du système. 
    > Par conséquent, nous avons besoin des droits d'administeurs, obtenus avec la commande sudo.


## Lecture de la valeur d'une entrée GPIO
- Pour lire la valeur du bouton poussoir il faut faire : 
 > gpio_mmap ( (void **)&gpio_regs_virt ) < 0
 Dans la fonction read bp. 
- Et dans la fonction gpio_read(pin) nous testons la valeur du bouton poussoir, s'il est appuyé, nous modifions la valeur des variables globales BP_ON et BP_OFF. 
- Dans la fonction write_bp, on test la valeur du bouton poussoir en bloucle suivi d'un court délai pour éviter les rebonds du bouton poussoir. 
 

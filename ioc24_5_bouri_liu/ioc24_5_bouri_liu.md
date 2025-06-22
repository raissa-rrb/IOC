## Serveur WEB minimaliste
>BOURI Raïssa
LIU Owen

>- **Objectif**
    - créer un site web consultable par browser permettatn d'accéder à des capteurs


- Le serveur web reçoit des requêtes du client, à leur récéption il exécute des scripts CGI ( **C**ommon **G**ateway **I**interface) pour produire des pages html dynamiques
- Ces scripts doivent communiquer avec notre programme écrit en C pour contrôler les LEDs
- Cette communication scripts/programme est faite à l'aide d'une FIFO

- **Étapes**
    1. Faire communiquer un programme python avec un programme C par FIFO
    2. Créer un serveur local sur le PC et le faire commmuniquer avec programme C


#### write.py

> - **Dans quel répertoire est créee la fifo ?**
Elle est créee dans le répertoire /tmp : **'/tmp/myfifo'**

```
pipe_name = '/tmp/myfifo'

if not os.path.exists(pipe_name):
    os.mkfifo(pipe_name)

pipe_out = open(pipe_name,'w')
```

> - **Quelle différence entre *mkfifo* et *open* **? 
**mkfifo** va permettre de créer un fichier spécial pour les fifo. Une fois crée, n'importe quel processus pourra y accéder. 
Avec mkfifo, la fifo sera créee dans le **système de fichier** mais aucune de ses extrémités ne sera ouverte dans notre processus.

C'est **open** qui permet de relier une de ses extrémités à notre processus.
Le programme écrivain devra ouvrir cette fifo avec les droits **O_WRONLY** et le programme lecteur avec **O_RDONLY**. 

> -Pourquoi tester que la fifo existe ? 

Dans la première partie du programme on vérifie à l'aide de **os.path.exists(path)** si la fifo **pipe_name** existe ou non. 

Dans notre cas, il faut que ce fichier spécial ne soit pas existant. 
S'il existe c'est qu'une fifo avec le nom **myfifo** existe déjà. 

>- **À quoi sert le flush?**
Lors d'un **write()**, par soucis de vitesse, les informations écrites sont stockées dans un **buffer interne** avant d'être écrite dans nos fichiers, ici, dans notre tube. 
Cela veut dire qu'en cas de fermeture du processus ou de perte de l'alimentation de l'ordinateur, les données du buffer seront perdues. 
Pour éviter cela, on peut utiliser **flush()** pour écrire les données contenues dans le buffer dans le tube ou fichier.

>- **Pourquoi ne ferme-t-on pas la fifo ?**
Si on l'a ferme on ne pourra plus l'ouvrir. 

#### reader.py

>- **Que fait readline?**
**readline()** permet de lire la première ligne d'un fichier, ici, elle lira la première ligne de notre fifo.

>- **Pourquoi rien ne se passe tant qu'un lecteur n'est pas ouvert?**
Parce que **open()** est un appel bloquant, il attend que les deux extrémités soient connectés avant de pouvoir écrire dans le tube. 
Cela pemet d'éviter les pertes de données par exemple, si des données sont écrites sans que personne ne les reçoie. 

### serveur fake 

- **Objectif**
    - fake **lit** une valeur sur stdin et la place dans une variable 
        - si plusieurs écritures successives alors on écrase la variable stockée
    - fake tourne en boucle
    - fake attends aussi un message de la fifo **s2f**
        - lors de la réception d'un message il l'affiche et envoie dans **f2s** la valeur lue sur stdin

- **Modifier select dans fake**
 **select** permet de sélectionner les différents descripteurs que l'on veut lire, on peut aussi définir plusieurs descripteurs sur lesquels écrire. 
Pour cela, on définit un **fd_set** qui va contenir nos différents descripteurs. 
On va utiliser les macros qui permettent de : 
    - vider un set : **FD_ZERO**
    - ajouter ou retirer un descripteur dans un set : **FD_SET** ou **FD_CLR**
    - tester si une entrée du set a reçu un résultat : **FD_ISSET**

Dans notre cas, on veut ajouter un dedscripteur sur **stdin**

```FD_SET(STDIN_FILENO, &rfds);```

De cette façon, **fake** surveille aussi les entrées sur stdin, mais il faut aussi vérifier qu'une entrée attendue soit "active" et d'envoyer le contenu lu vers **f2s**
```        
if (FD_ISSET(STDIN_FILENO, &rfds)) {                     // something to read
    int nbchar;
    if ((nbchar = read(STDIN_FILENO, serverRequest, MAXServerResquest)) == 0) break;
        printf("lu : %s \n",serverRequest );
        serverRequest[nbchar]=0;
        write(f2s, serverRequest, nbchar);

    }
}
```

- **Modifier server.py pour lire la valeur lue**
Ici, il va falloir lire **f2s** et afficher son contenu.

```
str = f2s.readline()
print(str)
```
 
 ##Accès aux leds et au bouton poussoir par le serveur

 ### Modification de fake.c 
À présent on veut que fake.c **allume** ou **éteigne** les LEDs.
Tout d'abord on ouvrre notre driver de led  
```
    int fd_led_BL = open("/dev/led_s", O_WRONLY);
    if(fd_led_BL < 0){
        fprintf(stderr, "Erreur d'ouverture du pilote LEDs et Bouton\n");
        exit(1);
    }

```
Ensuite, on teste la valeur reçu par **f2s**: 
    - Si on reçoit **w 1** on envoie **1** vers le driver de led
    - Si on reçoit **w 0** on envoie **0** 
```
if(!strcmp(serverRequest,"w 1\n")){
    printf("On\n");
    write(fd_led_BL, "1", 1);
    continue;
}
 if(!strcmp(serverRequest,"w 0\n")){
     printf("Off\n");
    write(fd_led_BL, "0", 1);
    continue;
}
```
 

 

 







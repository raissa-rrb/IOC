### Lab 6 : Mise en oeuvre du modèle client-serveur
- Bouri Raïssa
- Liu Owen
#### Compréhension des codes serveur et client
##### 1) Commentaire du code server.c

```
if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
}
```

Le code débute avec ce test qui vérifie que le numéro de port a bien été donné en paramètre. 

```
sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
```
Puis, on créer la socket avec les paramètres suivants : 
- AF_INET : pour le  domaine, cela signifie que la communication entre le client et le serveur se fait entre processus sur différents hôtes connectés en IPV6. 
- SOCK_STREAM : pour le type de communicaton, ici, TCP qui est un type de connection sécurisé comparé à UDP. 
- 0 : pour le protocol, la valeur 0 correspond au protocol Internet. 

La fonction **socket(...)** retourne un entier qui correspond au descripteur de la socket, c'est cela que l'on utilisera pour y écrire ou lire. 
Pour être propre on n'oublie pas de tester sa valeur pour éviter toute erreur. 

```
portno = atoi(argv[1]);
```
On attribue à notre variable **portno** le numéro de port passé en paramètre. 
```
struct sockaddr_in serv_addr
bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = INADDR_ANY;
serv_addr.sin_port = htons(portno);
```
**bzero(void \*s, size_t n)** est une fonction qui permet de mettre à 0 les n premiers octets du bloc pointé par s. 
C'est une façon de nettoyer la mémoire, comme un memset. 
Ici, on met à 0 les champs de la structure serv_addr.
La structure **serv_addr** est de type **sockaddr_in**, c'est la structure pour les sockets de types INET, elle contient les champs :
- **sin_family** : permet de de spécifier le domaine de la socket, ici AF_INET comme spécifié lors de la création du descripteur de la socket.
- **sin_port** : permet d'attribuer le numéro de port 
- **sin_addr** : permet d'attribuer l'adresse IP

Dans notre code, la valeur de l'adresse IP est **INADDR_ANY**, cela signifie que la socket est associée à toutes les adresses possible de la machine. 

Pour le numéro de port, il y a une conversion à faire, en effet, il y a une normalisation des adresses et l'ordre dans lequel les octets sont stockés en mémoire, on parle de format Big Endian ou Little Endian. 
**htons** -*host to network*- permet de faire la conversion du format**host** vers le format **réseau, network**.

```
if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
```
Suite à la création de la socket, il faut **lier** la socket à l'adresse et port spécfier dans la structure sockaddr_in.
**bind(...)** prend en paramètres le descripteur de la socket, la taille de la structure sockaddr_in ainsi que la structure sockaddr_in, castée en **sockaddr\***. 
En fait, la structure **sockaddr** est la strucure "mère", c'est une sorte de classe abstraite de laquelle dérivent les classes **sockaddr_in** utilsées pour les sockets INET et **socketaddr_un** utilisées pour les sockets Unix. 
Comme cette opération de *liage* est commune à ces deux types de sockets, le type demandé est le type de base. 

Cette opération retourne 0 en cas de **succès** sinon une erreur s'est produite. 
```
listen(sockfd, 5);
```
**listen(int sockfd, int nb)** prend en paramètre le descripteur de la socket qu'on va lire et le nombre de connexions pendantes que l'on autorise. 
Cette fonction permet de mettre la socket en attente de connexion d'un client.

```
while (1) {
newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
```
Lorsque l'on a au moins une connexion, on créer une nouvelle socket d'acceptation de connexion. 
Celle-ci va extraire la première connection de la file d'attent et retourne le descripteur de cette socket. 
À ce stade, la connection client-serveur est établie et il est possible de transférer des donnnées.
Une fois encore, on vérifie bien qu'aucune erreur n'est retournée. 


```
bzero(buffer, 256);
n = read(newsockfd, buffer, 255);
if (n < 0)
    error("ERROR reading from socket");
```
Pour que le buffer soit propre on le met à 0 et dans la variable **n** on récupère le nombre d'octets lu par la fonction **read**, si ce nombre est inférieur à 0, c'est qu'une erreur s'est produite. 
**read** lit dans le descripteur de la socket de connection, et écrit les informations récupérées dans le buffer. 

```
printf("Received packet from %s:%d\nData: [%s]\n\n",                net_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),
buffer);

close(newsockfd);
```
On affiche ensuite les données reçues. 
Comme lors de l'attribution du numéro de port dans la structure **sockaddr**, il y a une conversion de format à faire pour les adresse IP. 
On utilise la fonction **inet_nota** pour récupérer l'adresse IP sous format chaine de caractères.

Une fois les données récupérées on peut fermer la socket de connection et reboucler. 
```
close(sockfd);
```
On n'oublie pas de fermer notre première socket. 

> **Comme vu en cours, un serveur créer une socket de demande d'écoute qu'il lie à un port, il écoute le réseau en attente de demande de connection, une fois qu'il y en a il accepte ses connectione t créer une seconde socket pour effectuer les transfert client--serveur et lit sur celle-ci pour récupérer les données.**

##### 2) Commentaire du code client.c

```
if (argc < 3) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    exit(0);
}
portno = atoi(argv[2]);

```
On commence par récupérer l'adresse IP du serveur auquel le client doit se connecter ainsi que le numéro de port. 
S'ils ne sont pas transmi, on termine sur une erreur. 

```
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0)
   error("ERROR opening socket");
```
Comme pour le serveur, on créer la socket TCP et on récupère son descripteur. 

```
struct hostent *server;
server = gethostbyname(argv[1]);
if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
}
```
**gethostbyname** prend en paramètre l'adresse IP à laquelle on veut connecter la socket, elle récupère des informations sur l'hôte. Ces informations sont placées dans la structure **hostent**. 
On y trouvera donc le **nom** de l'hôte, ainsi que sa structure **in_addr** qui représente l'adresse internet. 
En cas d'erreur elle renvoie un pointeur null. 

```
bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
serv_addr.sin_port = htons(portno);
```
Comme vu avec le serveur, ona ttribue au champ **sin_family** le domaine de la socket : AF_INET. 
On copie ensuite les informations de l'hôte dans la structure **in_addr**. 

```
if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
   error("ERROR connecting");
```
L'étape qui differt entre les serrveurs et clients et **connect**. 
Celle-ci permet d'initier la demande de connection vers un serveur.
Cet appel est bloquant, on ne peut pas envoyer de données tant que la connection n'est pas établie. 
Parallèlement, le serveur est *en attente* de connection, avec la fonction **listen**. 

```
strcpy(buffer,"Coucou Peri\n");
n= write(sockfd,buffer,strlen(buffer));
if (n != strlen(buffer)
    error("ERROR message not fully trasmetted");
```
Une fois la connection établie, on peut écrire dans la socket avec un **write**. 
On s'assure que le nombre d'octets écrit correspond bien à la taille de notre buffer, sinon une erreur s'est produite. 

```
close(sockfd);
return 0;
```
On termine proprement en fermant la socket :alien:

> **Pour résumer, cet exemple nous illustre bien les étapes principales d'une socket client.**
> **Comme le serveur elle créer sa socket qui lui permettra de se connecter et de transmettre des données, une fois connectée au serveur et émet ses données en écrivant dans le descripteur de socket et se ferme une fois le transfert terminé.**

#### Création d'une application de vote.
Ce que l'on veut:
- Recueuillir le vote d'une personne
    - choix de l'heure d'été ou d'hiver
- Un programme pour **voter** : le client
- Un programme pour **recueillir** le vote : le serveur
- **Attention** : il faut un mécanisme qui permet de vérifier que chaque personne ne pourra voter qu'**une seule fois.**

>Plan d'attaque 
> - Je commence par simplement copier les exemples de codes ci-dessus pour avoir le squelette de base. 
> - J'implémente un mécanisme de vérification des personnes qui votent 
    - une hashmap fera l'affaire
    - c'est le serveur qui se charge de ça
        - en fait on a juste vérifier si la personne est dans la table de hash, les participants ne s'enregistrent pas avant de voter, donc je vérifie juste qu'ils soient bien enregistré et ça signifiera qu'ils ont déjà voté
> - Ensuite on complète pour avoir le comportement demandé, il faut comptabiliser les votes.

##### Implémentaton de la hashMap

Dans notre hashmap, les **clés** seront les noms des participants au vote, et les **valeurs** une informaton indiquant qu'ils aient déjà voté ou pas, je vais partir sur un int : 
- 0 : n'a pas encore voté
- 1 : a déjà voté

S'il y a un mécanisme de vérification des votes, **il ne faudra pas oublier de mettre à jour cette tableà chaque vote**.

*il n'y a pas de hashmap dans la librairie standard en C donc il va falloir l'implémenter :upside_down_face:*

Pour l'implémenter ses fonctionnalités principales sont : 
- L'insertion de clés
- La mise à jour des valeurs associées aux clés
- La recherche de clés
- La fonction de hashage
```
typedef struct hash_item {

    char* key; //voter's name
    int   value; //0 :  winter / 1 : summer / -1 : didn't vote
    struct hash_item* next; //pointer to the next item
    
} hash_item;
```
On commence par décrire la structure **item**, ce sont ces structures que l'on placera dans la HashTable. et elles seront chainées entre elles. 
Le champ value contiendra donc : 
- 0 : si la personne a voté **hiver**
- 1 : si la personne a voté **été**
- -1 : s'il la personne n'a **pas voté**

À présent, oon peut implémenter la table de Hash, celle-ci doit contenir des **h_item** chainés entre eux, une taille qui représente sa capacité maximale et le nombre d'éléments qui y sont présents.

```
typedef struct hashTable {
    
    size_t size; //capacité max
    size_t elem; //current number of elements
    hash_item ** item; // pointer to hash_item
} hashTable;
```

À présent que j'ai une structure pour **stocker** les données des participants et et une HashMap, j'ai besoin de fonctions pour **allouer** la mémoire, **initialiser** mes structures et les **chainer**.

#### 1) create_item, create_table et free
```
hash_item* create_item(char * key, int value)
{
    /* memory allocation for the item */
    hash_item* item = (hash_item) malloc(sizeof(hash_item));

    /* allocates strlen + 1 for '\n */
    item->key = (char*) malloc(strlen(key)+1);

    item->next = (hash_item) malloc(sizeof(hash_item));

    /* copy of given key into field key of the item */
    strcpy(item->key, key);

    item->value = value;

    item->next = NULL;

    return item;

}
```
On alloue bien la mémoire des champs et on y met les paramètres. 

```
 hashTable* create_table(int size)
 {
    /* memory allocation for the table */
    hash_table* hTable =  (hashTable) malloc(sizeof(hashTable));
    /* ini fields size and nb_elem */
    hTable->size = size;
    hTable->nb_elem = 0;
    /* allocates memory for the items, we have size items */
    hTable->item = (struct hash_item**) malloc(sizeof(struct hash_item*)*size);

    /*set items to null for clean space*/
    for(int i =0; i < size; i++){
        hTable->item[i] = NULL;
    }

    return hTable;
 }
```
Comme précedemment on alloue la mémoire pour la table, ses champs et les items qu'elle va contenir. 
On initialise les champs size et nb_elmem aux bonnens valeurs et pour commencer proprement on initialise toutes les structures à null. 

La règle d'or en C : un malloc = un free. 
On va implémenter les fonctions de libération de la mémoire dès maintenant.

```
void free_item(hash_item* item)
{
    free(item->key);
    if(item->next)
        free(item->next);
    free(item);
}

void free_table(hashTable* table)
{
    /* free all the items it contains */
    for(int i =0; i < table->size; i++){
        if(table->item[i] != NULL)
            free(table->item[i]);
    }

    /* then we can free the fields */
    free(table->item);
    free(table);

}
```
On désalloue donc chaque champs alloué. 

#### 2) Fonction de hashage
Il nous faut maintenant une fonction de hashage qui va déterminer comment nos clés sont placés dans la table. 
J'ai pris une fonction de hashage toruvée sur internet utilisée pour hasher des chaines decharactères.


#### 3) add_item
C'est pendant l'inserton d'un item qu'on peut vérifier si la personne a déjà voté, si elle y est déjà, c'est qu'elle a déjà voté.
Donc, ma fonction **add_item** va retourner un entier : 
- 0 : insertion ok
- 1 : la clé est déjà dans la table

```
int add_item(hasTable* table, char* key, int value)
{
    /* create the item */
    hash_item* item = create_item(key,value);
    
    /* item's index */
    int index = hash_function(table,key);

    hash_item* check_item = table->item[index];

    /* if empty at this index then we create new item */
    if(check_item ==  NULL){
        table->item[index] = item;
        return 0;
    }

    else {
        /* if not null check if it's the same */
        if( strcmp(check_item->key , key) == 0 )
            return 1; // item already in the table
        
        else{
            /* it's not the head let's go through it */
            check_item = check_item->next;
            while(check_item){
                if( strcmp(check_item->key, key))
                    return 1;
                check_item = check_item->next;
            }
            /* if we get here that means it's not in the table so we add it at the head */
            item->next = table->item[index]; //link the new and the previous head
            table_item[index] = item; //add new one at the head
            return 0;

        }
    }
}
```
Donc dans l'ordre : 
1. Je créer l'item à insérer
2. Je calcule l'index de l'item selon une fonction de hashage
3. Je récupère la case correspondant à cet index
    4. S'il n'y a pas d'item, cette ligne de la table est vide, donc j'ajoute mon item
    5. Si il y a un item je vérifie que ça n'est pas celui qui vient de voter, si oui je retourne 1 ( a déjà voté)
    6. Si ce n'est pas le même que le mien, je parcour la ligne de la table à la recherche de mon participant 
        7. Si je le trouve, je retourne 1 : a déjà voté
        8. Si je ne le trouve pas je le met en tête de la ligne et je retourne 0 : n'a pas encore voté.

#### 4) Recherche d'un item dans la table 
Ça va reprendre des élèments du code précédent, par soucis de propreté j'écrirais peut être une fonction de recherche, en attendant je vais juste réécrire ça. 
On va juste calculer l'index et parcourir la table à la recherche de l'élément. 
Cette fois-ci on retournera juste la valeur du vote, et -1 sinon.

```
int search_item(struct hashTable* table, char* key )
{
    int index = hash_function(table, key);

    hash_item* cur = table->item[index]; 

    while(cur){
        if( strcmp(cur->key, key) == 0)
            return cur->value;

        cur = cur->next;
    }
    return -1;
}
```

#### 5) Comportement du serveur
Maintenant que la hashmap est fonctionnelle, il faut coder le bon comportement du serveur. 

En plus de toutes ses étapes de connection, il doit : 
- Récupérer le nom des électeurs
- Récupérer leur vote
- Vérifier qu'ils n'ont pas déjà voter
    - S'ils ont déjà voter, message d'erreur
    - Sinon les ajouter dans la base de donnée

Donc j'ai décidé de concatainer nom et vote et de l'envoyer tel quel à la socket. 
Une fois le message reçu, la socket parcours le buffer à la recher de l'espace pour séparer la valeur vote et nom et les met dans leur propre buffer. 
Ces buffers sont ensuites insérés dans la hashmap. 
*Il y a surement un moyen plus simple mais c'est ce à quoi j'ai pensé en premier* :woman-shrugging: 

```
strcpy(buffer, argv[3]);
strcat(buffer," ");
strcat(buffer,argv[4]);
```
Je concataine les deux chaines en les séparant par un espace

```
 for(i = 0; buffer[i] != '\0'; i++){
    if(buffer[i] != ' ' && name == 0){
        nom[i] = buffer[i];
}
    if(buffer[i] == ' '){
        name += 1;
}
   if(buffer[i] != ' ' && name==1){
         vote[j] = buffer[i];
         j++;
    }    
}
```
À chaque récéption, on copie le nom dans son buffer. 
Une fois arrivé au caractère espace, on met à 1 la variable **nom** pour passer à la lecture du vote. 

```

if(strcmp(vote,"1") != 0 && strcmp(vote,"0") != 0){
    printf("incorrect vote value !\n");
 }
else {
    if(add_item(table,nom,vote)==0){
        printf("vote registered !\n");
        print_table(table);
}
                        
}
```
On vérifie que la valeur de vote est correcte : soit hiver soit été. 
Si elle l'est on peut ajouter le vote de la personne dans la table 

***Si ce n'est pas efficace et q'il y avait une meilleure façon de faire c'est parce que je fais le ramadan et que je manque de sucre*** :face_with_rolling_eyes: 

![Capture d’écran du 2024-03-16 00-23-23](https://hackmd.io/_uploads/r17i_LzCT.png)

```
./client localhost 32000 raissa 0
```
```
Hi please vote 
'0' for winter or '1' for summer
nom :raissa 	 vote: 0
vote registered !
****** start printing table ******
 key : raissa 	 value : 0 
****** end of table ******
Received packet from 0.0.0.0:0
Data: [raissa 0]
```

```
./client localhost 32000 raissa 1
```
```
Hi please vote 
'0' for winter or '1' for summer
nom :raissa 	 vote: 1
you already voted 
Received packet from 127.0.0.1:34526
Data: [raissa 1]
```

```
./client localhost 32000 rania 1
```
```
Hi please vote 
'0' for winter or '1' for summer
nom :rania 	 vote: 1
vote registered !
****** start printing table ******
 key : rania 	 value : 1 
 key : raissa 	 value : 0 
****** end of table ******
Received packet from 127.0.0.1:34530
Data: [rania 1]
```

```
./client localhost 32000 maman 90
```
```
Hi please vote 
'0' for winter or '1' for summer
nom :maman 	 vote: 90
incorrect vote value !
Received packet from 127.0.0.1:39312
Data: [maman 90]
```

```
./client localhost 32000 maman 1
```
```
Hi please vote 
'0' for winter or '1' for summer
nom :maman 	 vote: 1
vote registered !
****** start printing table ******
 key : rania 	 value : 1 
 key : raissa 	 value : 0 
 key : maman 	 value : 1 
****** end of table ******
Received packet from 127.0.0.1:57604
Data: [maman 1]
```

:alien: :alien: :alien: :alien: :alien: :alien: :alien: :alien: :alien:
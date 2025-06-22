### Lab 4: ProgrammationESP32 / Arduino

>- Objectif : 
    - Écrire une application multitâches Arduino

>- Pour moi : 
        >problèmes que j'ai rencontré lors de l'installation sur mon pc : 
            - **python not in path**-> faire un lien symbolique python3 et python 
            - **serial not found** -> pip3 install pyserial
            - **usbtty0 permission denied** -> chmod a+rw /dev/ttyUSB0


#### Programmation multi-tâches
1. #### Taches
Pour chaque tâche nous avons : 
- une fonctoin **loop_tache()** : elle code le comportement de la tâche et est appelée par **loop()**
- une fonction **setup_tache()** : elle initialise les périphériques, ressources de la tâche et l'état interne
- une structure : elle contient l'état interne et le contexte d'éxécution de la tâche
    - Le contexte d'éxécution est représenté par une variable globale sous forme d'une structure **Tache_t**
    - Cette structure est passée en arguement des fonction **ssetup_Tache()** et **loop_tache()**
- Concernant **setup_tache** et **loop_tache**, entre deux instances d'éxécutions, leurs variables locales ne sont pas conservées.
Elles peuvent également avoir des variables statics qui ont une valeur unique. 
Dans le cas où la tache est en plusieurs exemplaire et que ces exemplaires utilisent la même fonctoin **loop_tache** elles auront une valeur uniques aussi.
D'ailleurs, l'utilisation de ces variables statics n'est pas conseillées car elle ne peuvent pas être initialisées dans la fonction **setup_Tache**.
Puisqu'elles ont un valeur unique, on ne pourra pas les initiliaser pour chaque taches s'il y en a plusieurs. 


```
struct Tache_s {
  unsigned int etat;  
  int config;        
}; 
struct Tache_s T1, T2;
```
La structure **contexte**. 
Pour des taches ayant le même comportement on instancie autant de structures que de taches.

Cette structure est initialisée par la fonction **setup_Tache()**
```
void setup_Tache(struct Tache_s *ctx, params...) {
   // Initialisation du contexte}
   ctx->etat = etat_initial;  //  reçu dans les paramètres
   ...
}
```
```
void loop_Tache(tache_t *ctx, connectors....) {   
   // test de la condition d'exécution, si absent on SORT
   if (evement_attendu_absent) return;
   // code de la tache
   ....
}
```
La fonction **loop**, exécute donc les fonctions **loop_tache** à tour de rôle. 
Pour éviter les famines, les tâches n'ont pas le droit de conserver le processeur e, faisant des boucles d'attentes d'un évènement. 

On voit que dans les paramètres de **loop_tache** on trouve **connectors**.
Ce sont des pointeurs vers des variables globales. Celles-ci sont utuilisées pour la communication inter-taches. 

2. #### Gestion des tâches périodiques

Pour les tâches périodiques on utilise un **timer interne** du processeur qui s'incrémente chaque seconde. 
Ce timer est exploité dans la fonction **waitFor(int timer, unsigned long period**.
- **timer** : un numéro de timer, un identifiant, il en faut autant que de taches.
Il est défini dans le **#define MAX_WAIT_FOR_TIMER**
- **period** : une période en microsecondes

Cette fonction peut être appelée plusieurs fois et renvoie 1 une fois par période. 
Si elle n'est pas appelée pendant un moment alors elle rend le nombre de périodes écoulées.

3. #### Modification de l'exemple
```
salut
bonjour
bonjour
salut
bonjour
bonjour
salut
bonjour
```
J'ai modifié le code pour avoir **bonjour** qui s'affiche toute les secondes et **salut** toute les 2 secondes. 
Pour cela, j'ai instancié une nouvelle structure **Mess**.
```
struct Mess_s Mess1, Mess2;
```
Et ajouté **setup_Mess** pour la tache **Mess2** dans **setup()** et dans **loop**
```
void setup() {
  setup_Led(&Led1, 0, 100000, LED_BUILTIN);                        // Led est exécutée toutes les 100ms 
  setup_Mess(&Mess1, 1, 1000000, "bonjour");                      // Mess est exécutée toutes les secondes 
  setup_Mess(&Mess2, 2,  2000000, "salut");
}

void loop() {
  loop_Led(&Led1);                                        
  loop_Mess(&Mess1); 
  loop_Mess(&Mess2); 
}
```

3. #### **Questions**
> - Que contient le tableau waitForTimer[] et à quoi sert-il ? 
> 
Ce tableau a la **taille du nombre de timer maximum** que l'on peut avoir dans le programme : un par tâches périodiques
Il contient le nombre de période à attendre pour chaque timer. 

>  - Si on a deux tâches indépendantes avec la même période, pourquoi ne peut-on pas utiliser le même timer dans waitFor()? 

Après essai, on obtient un résultat très hétérogène on obtient **bonjour** pendant un certains temps puis **salut** pendant ce même temps puis **bonjour** suivit de **salut** et on recommence. 

Si on utilise le même timer pour deux tâches, la première va appeler **waitFor** en premier, et celle-i va mettre à jour le nombre de période à attendre. La seconde fera de même. 
Sauf que lorsque le nombre de période attendu sera arrivé, c'est la tâche qui aura appelé **waitFor** en premier qui pourra exécuter son code. 
Celle qui l'appellera en second, se rerouvera avev un delta < 0 et devra attendre encore. 

On aurait pu s'attendre à ce qu'elles éxcutent leur code *en même temps* sauf que ça n'est pas possible puisque nous avons seulemnt une **illusion** de parallélisme. 

> - Dans quel cas **waitFor()** peut rendre 2? 

Si elle n'est pas appelée pendant un moment alors lors de son appel elle rendra le nombre de périodes écoulées. 
Par exemple, si on l'appelle avec une période de 100, qu'on ne l'appelle plus pendant 200microS, et qu'au bout de ce d"lai on l'appelle, elle retournera 2: qui représente le nombre de période écoulées entre le premier de dernier appel

4. **Utilisation de l'écran OLED** 

On va créer une tâche **Cpt** qui va incrémenter un compteur toute les secondes et l'afficher sur l'écran. 

Tout d'abord, il nous faut une fonction **waitFor**
```
#define MAX_WAIT_FOR_TIMER 1
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches périodiques
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta = 1 + newTime;                   // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}
```
Ensuite, il nous faut une structure pour la tâche **cpt**
```
struct Cpt_s {
  int timer;             
  unsigned long period;      
  int val;
};
```
Il nous faut également une instance de cette tâche.
Et ses fonctions **loop_Cpt** et **setup_cpt**

```
void setup_Cpt( struct Cpt_s * ctx, int timer, unsigned long period, int val) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->val = val;
  Serial.begin(9600);                                     // initialisation du débit de la liaison série
}

void loop_Cpt(struct Cpt_s *ctx) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
  ctx->val +=1;
 // Serial.println(ctx->val);                              // affichage du message

  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println((ctx->val));
  display.display();

}
```

- **setup_cpt** initilise les valeurs du timer, de la période et de la valeur initiale du compteur. 
- **loop_cpt** définit le comportement du compteur. 
En étudiant le code de setup du fichier test, j'ai vu l'existence d'une fonctoin **testscrolltext** qui affichait le text *scroll* sur l'écran OLED. 
J'ai donc pris la partie qui affiche le test. 
On **clear** d'abord l'écran pour qu'il soit vide. 
On sélection une taille de texte et une couleur avec les fonctions **setTextSize** et **setTextColor**. 
Comme pour l'écran LCD des raspberry nousa vons une fonction **setCursor** qui permet de modifier l'emplacement du text sur l'écran. 
Et enfin la fonctoin **println** qui permet d'afficher du text sur l'écran.
Cette fonction permet également d'incrémenter le compteur à chaque seconde. 

La fonction **setup** doit appeler **setup_Cpt**. 
Et **loop** appelle **loop_cpt**.

```
void setup() {
  Serial.begin(9600);
  Wire.begin(4, 15); // pins SDA , SCL

  setup_Cpt(&Cpt1, 0, 1000000, 0); 

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
} 

void loop() {

  loop_Cpt(&Cpt1); 
}

```
5. **Communication inter-tâches**

Pour la communication inter-taches nous allons utiliser des variables globales ainsi qu'une **boite à lettres**.

Soient deux taches, T1 et T2. 
Si T1 est l'écrivain alors il pourra envoyer des données via la boite que si celle-ci est **vide**.
Inversement, T2, le lecteur, devra attendre que la boite soit **pleine** pour y lire les données.
```
enum {EMPTY, FULL};

struct mailbox_s {
  int state;
  int val;
};

struct mailbox_s mb = {.state = EMPTY};
```
Notre boite à lettres possède une **variable d'état** **: state** qui prend soit la valeur **EMPTY** soit la valeur **FULL**. 

```
void loop_T1(... mailbox_t * mb ...) {
  if (mb->state != EMPTY) return; // attend que la mailbox soit vide
  mb->val = 42;
  mb->state = FULL;
}

void loop_T2(... mailbox_t * mb ...) {
  if (mb->state != FULL) return; // attend que la mailbox soit pleine
  // usage de mb->val
  mb->state = EMPTY;
}
```
Ci-dessus, les deux loop de nos tâches T1 et T2. 
Elles communiquent via la boite **struct mailbox_s mb** dont l'état est initialisé à **EMPTY**.
Cela signifie que seul la tâche écrivaine peut y accéder. 
Les deux tâches doivent prendre la boite à lettres en argument.
T1, la tâche écrivaine, **teste** l'état de la boite, si elle est vide alors elle peut y écrire une valeur et paser l'état de la boite à pleine, sinon elle termine.

Inversement, T2, la tâche lectrice, teste l'état de la boite, si elle est pleine alors elle peut lire la valeur stockée et passer l'état de la boite à vide.

>**Questions**
>Ajouter une tâche nommée lum qui lit toutes les 0,5 seconde le port analogique [...] (par analogRead()) sur lequel se trouve la photo-résistance et qui sort sa valeur dans une boite à lettre. Cette boite à lettre sera connectée à la tâche oled. Vous afficher la valeur en pourcentage de 0% à 100% en utilisant la fonction map() 

Comme précedemment, nous allons instancier une structure pour la tâche **lum**, ainsi que ses fonctions **loop_lum** et **setup_lum**

```
truct Lum_s {
  int timer;             
  unsigned long period;
};

//boite à lettre
enum {EMPTY, FULL}; 

struct mailbox_s {
  int state; 
  int value;
};

struct Oled_s Oled1;
struct Lum_s lum;
struct mailbox_s mb = { .state = EMPTY};
```
Donc, nous avons une nouvelle tâche **Lum** qui ne prend pas de valeur, ainsi qu'une structure **maibox** comme vue dans l'énoncé. 

```
void loop_Oled(struct Oled_s *ctx, struct mailbox_s *mb) {

   if( mb->state != FULL ) return;
   ctx->val = mb->value;

   mb->state = EMPTY;
  
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println((ctx->val));
  display.display();

}
```
À présent, **loop_oled**, la tâche lectrice, vérifié que la boite à lettre soit pleine, dans ce cas elle y récupère la valeur de la photrésistance et l'affiche sur l'écran.

**loop_lum** procède de la même manière, elle vérifie que la boite soit vide et y écrit la valeur de la photorésistance sous forme de pourcentage.

Pour ce faire, elle utilise la fonction **map()**. 
Celle-ci prend en arguement:
- la **valeur à lire** : ici, **analog_read(36)**, la photorésistance
- la valeur **minimale de la photorésistance** : 0
- la valeur **maximale de la photorésistance** : 4095 d'après mes tests
- la valeur **minimale que l'on souhaite avoir** : 0%
- la valeur **maximale que l'on veut ** : 100%


> Mofifier la tâche Led pour que la fréquence de clignotement soit inversement proportionnel à la lumière reçue (moins il y a de lumière plus elle clignote vite). La tâche Led devra donc se brancher avec la tâche lum avec une nouvelle boite à lettre. Il doit y avoir deux boites sortant de lum, l'une vers oled l'autre vers led.

À présent on a une seconde boite à lettres 
```
struct mailbox_s mb2 = { .state = EMPTY};
```
Celle-ci est passée à **loop_lum**

```
void loop_lum(struct Lum_s *ctx, struct mailbox_s * mb, struct mailbox_s *mb2) {

  if( mb->state != EMPTY) return;
  //ne peut écrire que si la boite est vide
  mb->value = map(analogRead(36), 0, 4095, 0, 100 );
  
  //indique qu'elle a écrit
  mb->state = FULL;

  if( mb2->state != EMPTY) return;
  mb2->value = map(analogRead(36), 0, 4095, 0, 100 );
  mb2->state = FULL;
}
```
Qui comme pour la première boite à lettre, y écrit la valeur de la photorésistance. 

Pour la tâche **led**, j'ai ajouté une seconde varibale, **lum** celle-ci conserve la valeur de la période de base. 

```
void loop_Led( struct Led_s * ctx, struct mailbox_s *mb) {
  if (!waitFor(ctx->timer, ctx->period )) return;          // sort s'il y a moins d'une période écoulée
  digitalWrite(ctx->pin,ctx->etat);                       // ecriture
  ctx->etat = 1 - ctx->etat;   
  
  if( mb->state != FULL) return;                          // changement d'état
  //period inversement prop à la la lumiere
  ctx->period = ctx->lum * 1/((mb->value+1)/10);
  mb->state = EMPTY;
}
```
À chaque fois qu'une valeur est écrite dans la boite à lettre, **loop_led()** met à jour la valeur de sa période - *qu'il ne faut pas écraser, d'où la variable lum*.
Moins il y a de lumière : photorésistance = 100% plus la vitesse de clignotemment de la led est rapide. 
Et bien sûr on n'oublie pas le **setup_led** et **loop_led** dans les fonctions **setup** et **loop

6. **Gestion des interruptions**

Pour cette partie, nous allons utiliser la fonction **serialEvent()**.
D'après [Arduino SerialEvent example](https://docs.arduino.cc/built-in-examples/communication/SerialEvent/) cette fonction est appelée à chaque fois qu'une donnée est disponible sur le bus.

La fonction **attachInterrupt**, permet de tester en continu un évènement tout en permettant de faire d'autres taches, par exemple de tester l'appui d'un bouton ou dans  notre cas, l'envoi d'un caractère.

**7. Envoyer des données via Serial**

- **Serial.read()** : renvoie le premier octet du serial buffer.
- **Serial.available** : vérifie s'il y a des octets disponibles dans le serial buffer


Pour commencer j'ai simplement écrit une fonction **SerialEvent()** appelée par **loop** si jamais le **serial buffer** contient quelque chose. 

```
loop(){
....
if(Serial.available())
    SerialEvent(&mb3);
}
```
```
void SerialEvent(struct mailbox_s *mb){
  char lu = Serial.read(); 
  if(lu == 's'){
    Serial.println("j'ai reçu un s");
    mb->value = 1;
    mb->state = FULL;
  }
  else 
    Serial.println("c'est pas un s ça >:(");
}
```
**SerialEvent()** reçoit une mailbox en paramètre pour communiquer avec **led**.
Si jamais un 's' est envoyé alors elle met à 1 la variable contenue dans la mailbox. 
```
loop_led(){/
...
...
...
  if( mb3->state != FULL) return;
  if(mb3->value == 1){
    ctx->etat = 0;
    digitalWrite(ctx->pin,ctx->etat);
    mb3->state = EMPTY;
  }
```
**loop_led()** surveille cette mailbox et éteint la led si jamais la varible est à 1.

De cette façon je teste le fonctionnement de SerialEvent et de la communication via le serialMonitor. 

#### Utilisation du buzzer

[ESP32 buzzer with PWM](https://techtutorialsx.com/2017/07/01/esp32-arduino-controlling-a-buzzer-with-pwm/)

On va utiliser les fonctonnalités PWM de la **led**. 
Sur l'ESP32 nous avons accès aux fonctions **ledcstup()**, **ledcAttachPin()** et **ledcWrite()**. 
Celles-ci vont nous permettre de modifier le **duty cycle** ou encore la fréquence de notre buzzer ou led. 
En général, nous n'aurons pas besoin de modifier le duty cycle du buzzer, mais le modifier permet d'**augmenter son volume**. 
En modifiant sa **fréquence** nous pourrons modifier le type de son obtenu.

J'ai trouvé une librairie qui implémente les fonctions **tone()** et **noTone()** qui ne sont pas disponibles pour les esp32.
Elle utilise les fonctions PWM **ledcWrite()**, **ledcAttache()** etc...

- **ledcWriteTone()** : permet de positionner la fréquence du buzzer
- **ledcWrite()** : permet de positionner le duty cycle
- **lecsAttachPin()** : permet de relier le PWM à notre GPIO

Grâce à la librarie **Tone32.h**, nous avons juste à utiliser **tone()** et **noTone()**

```
void tone(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t channel)
{
    if (ledcRead(channel)) {
        log_e("Tone channel %d is already in use", channel);
        return;
    }
    ledcAttachPin(pin, channel);
    ledcWriteTone(channel, frequency);
    if (duration) {
        delay(duration);
        noTone(pin, channel);
    }    
}

void noTone(uint8_t pin, uint8_t channel)
{
    ledcDetachPin(pin);
    ledcWrite(channel, 0);
}
```

Pour jouer une mélodie, nous passons en arguments à **tone()** une fréquence correspondant à une note qui est définie dans **#define** dans **pitches.h**

```
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
....
....
int melody[] = {

  //Based on the arrangement at https://www.flutetunes.com/tunes.php?id=169
  
  NOTE_AS4,-2,  NOTE_F4,8,  NOTE_F4,8,  NOTE_AS4,8,//1
  NOTE_GS4,16,  NOTE_FS4,16,  NOTE_GS4,-2,
  NOTE_AS4,-2,  NOTE_FS4,8,  NOTE_FS4,8,  NOTE_AS4,8,
  NOTE_A4,16,  NOTE_G4,16,  NOTE_A4,-2,
  ....
 }:
....
tone(BUZZER_PIN, melody[i], noteDuration*0.9, BUZZER_CHANNEL);
```

#### Utilisation du BP


```
pinMode(buttonPin, INPUT_PULLUP);
```
Tout d'abord, il faut positionner le BP en mode **INPUT_PULLUP**. 
Donc si on n'appuie pas sur le BP sa valeur vaut *high* ou 1. 

Pour lire sa valeur on utilise la fonction **digitalRead(buttonPin)**.

#### Tout ensemble

Objectif : 
- texte qui clignote sur l'écran OLED : start puis choix
- communication serial : 
    -  si envoie de "link" il apparait à l'écran
    -  si envoie de "led" celle-ci clignote
- appuie BP : musique joue

2. **Clignotement on/off avec BP**

On va garder le comportement précédent de la LED : moins il y a de lumière plus elle clignote vite. 
Mais maintenant, si on appuie sur le bouton poussoir son clignotemment s'arrête, si on appuie une seconde fois son clignotement reprend.

Pour cela, on va utiliser **attachInterrup()**.
Cette fonctoni prend les paramètres suivants : 
- **interrupt** : ici, le pin du BP sur lequel on attend un changement d'état
- **ISR** : la fonction appelée en cas de changement d'état du BP, ici **blink()**
- mode : définit **quand** l'interruption doit être levée 
    - **LOW**
    - **CHANGE** : dès que le pin change
    - **RISING**: quand le pin passe de **LOW** à **HIGH**
    - **FALLING** : quand le pin passe de **HIGH** à **LOW**
    - **HIGH**
Ici, j'utilise **HIGH**, on veut que dès l'appui sur le BP la valeur de la LED soit modifiée.
```
void blink() {
  led.etat = !led.etat;
}

void setup() {

  setup_Led( &led , 0 , 100000 , LED_BUILTIN);


/* setup BP */
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, HIGH);
}
void loop() {
  loop_Led( &led );
}
```

Maintenant, on ajoute ça au mode de fonctionnement précèdent : la led clignoted selon la luminosité qui est affichée sur l'écran OLED.
Pour que l'appui sur le BP permettent d'arrêter le clignotement j'ai procédé exactement comme ci-dessus mais avec une varibable **interrupt** dans **Led_s** positionnée à 0. **blink()** la fait passer à 1 ou 0. 
Dans **loop_led()** si elle est à 1 l'état de la led vaut 0.
```
void blink() {
  led.interrupt = !led.interrupt;
}
```

```
loop_led() 
.....
if( ctx->interrupt == 1)  ctx->etat = 0;
  else
    ctx->etat = 1 - ctx->etat;  
  digitalWrite(LED_BUILTIN, ctx->etat);
```


#### Texte sur OLED avec BP

Maintenant, on veut que l'écran affiche **BP to stop** si la Led clignote ou **bp to start** si on l'a éteinte avec le BP. 

On va utiliser une nouvelle mailbox **mb_led_oled**. 
Si **interrupt** passe à 1 elle préviendra **oled**. 

```
loop-led()
/* led to oled for start/Stop msg */
  if( mb2->state == EMPTY){
    mb2->value = ctx->interrupt;
    mb2->state = FULL;
  }
```
```
loop_oled()
/* led clign on bp to stop */
  if( ctx->led_val == 0)
    display.print("stop");
  else 
    display.print("shine");

  /* led oled mb for start/stop msg */
  if( mb2->state != FULL) return;
  ctx->led_val = mb2->value;
  mb2->state = EMPTY;
```
#### Ajout du buzzer

À présent, on va refaire jouer le buzzer. 


Je vais donc modifier le prgramme pour qu'on puisse lancer la mélodie en tappant "p" pour **play** et "s" pour **stop**

Donc il nous faut une mailbox entre le serial event et le buzzer, et une seconde entre le serialevent et l'écran. 
```
   if( mb3->state == FULL){
      ctx->buzz_val = mb3->value;
      mb3->state = EMPTY;
  }
     if( ctx-> buzz_val == 1){
    display.clearDisplay();
    display.setTextSize(2); // Draw 2X-scale text
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(("listen...."));
    display.display();
    return;

  }
```

Pour pouvoir jouer la mélodie en multi-taches, j'utilise la fonction **millis()**, en effet **delay()**  a un effet bloquant. 

```
 unsigned long curM = millis();
   if(( curM - ctx->prevM) >= ctx->note_duration*0.9){
    ctx->prevM = curM;
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
  
    if( ctx->i < ctx->notes*2)
      ctx->i = ctx->i +2;
    else{
      ctx->i = 0;
    }
 
   }
```
**prevM** est une variable stockée dans la tâche Buzzer. 

De cette manière, pendant que la mélodie joue, les autres fonctonnalités fonctionnent.

![graph](https://hackmd.io/_uploads/BJLiIYwRa.png)

On a donc 4 mailbox. 
- lum qui envoie sa valeur à la led pour sa période de clignotement 
- lum qui envoie sa valeur à l'écran Oled pour afficher le pourcentage
- la led qui envoie sa valeur à l'écran pour la synchro avec le BP : bp start/stop
- la communication serial qui joue/arrête la mélodie du buzzer
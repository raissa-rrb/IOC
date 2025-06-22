#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Tone32.h"

// buzzer defines
#define BUZZER_PIN 17
#define BUZZER_CHANNEL 0

//oled defines
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define MAX_WAIT_FOR_TIMER 6
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches périodiques
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta = 1 + newTime;                   // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}

/* zelda NES theme */
int melody[] = {

  //Based on the arrangement at https://www.flutetunes.com/tunes.php?id=169
  
  NOTE_AS4,-2,  NOTE_F4,8,  NOTE_F4,8,  NOTE_AS4,8,//1
  NOTE_GS4,16,  NOTE_FS4,16,  NOTE_GS4,-2,
  NOTE_AS4,-2,  NOTE_FS4,8,  NOTE_FS4,8,  NOTE_AS4,8,
  NOTE_A4,16,  NOTE_G4,16,  NOTE_A4,-2,
  //REST,1, 

  NOTE_AS4,4,  NOTE_F4,-4,  NOTE_AS4,8,  NOTE_AS4,16,  NOTE_C5,16, NOTE_D5,16, NOTE_DS5,16,//7
  NOTE_F5,2,  NOTE_F5,8,  NOTE_F5,8,  NOTE_F5,8,  NOTE_FS5,16, NOTE_GS5,16,
  NOTE_AS5,-2,  NOTE_AS5,8,  NOTE_AS5,8,  NOTE_GS5,8,  NOTE_FS5,16,
  NOTE_GS5,-8,  NOTE_FS5,16,  NOTE_F5,2,  NOTE_F5,4, 

  NOTE_DS5,-8, NOTE_F5,16, NOTE_FS5,2, NOTE_F5,8, NOTE_DS5,8, //11
  NOTE_CS5,-8, NOTE_DS5,16, NOTE_F5,2, NOTE_DS5,8, NOTE_CS5,8,
  NOTE_C5,-8, NOTE_D5,16, NOTE_E5,2, NOTE_G5,8, 
  NOTE_F5,16, NOTE_F4,16, NOTE_F4,16, NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,8, NOTE_F4,16,NOTE_F4,8,

  NOTE_AS4,4,  NOTE_F4,-4,  NOTE_AS4,8,  NOTE_AS4,16,  NOTE_C5,16, NOTE_D5,16, NOTE_DS5,16,//15
  NOTE_F5,2,  NOTE_F5,8,  NOTE_F5,8,  NOTE_F5,8,  NOTE_FS5,16, NOTE_GS5,16,
  NOTE_AS5,-2, NOTE_CS6,4,
  NOTE_C6,4, NOTE_A5,2, NOTE_F5,4,
  NOTE_FS5,-2, NOTE_AS5,4,
  NOTE_A5,4, NOTE_F5,2, NOTE_F5,4,

  NOTE_FS5,-2, NOTE_AS5,4,
  NOTE_A5,4, NOTE_F5,2, NOTE_D5,4,
  NOTE_DS5,-2, NOTE_FS5,4,
  NOTE_F5,4, NOTE_CS5,2, NOTE_AS4,4,
  NOTE_C5,-8, NOTE_D5,16, NOTE_E5,2, NOTE_G5,8, 
  NOTE_F5,16, NOTE_F4,16, NOTE_F4,16, NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,16,NOTE_F4,8, NOTE_F4,16,NOTE_F4,8
  
};

/* mailbox */
enum {EMPTY, FULL}; 

struct mailbox_s {
  int state; 
  int value;
};

struct mailbox_s mb_lum_led = { .state = EMPTY};
struct mailbox_s mb_lum_Oled = { .state = EMPTY};
struct mailbox_s mb_led_Oled = { .state = EMPTY};
struct mailbox_s mb_serial_buzz = { .state = EMPTY};


/* tache buzzer_s */
struct Buzzer_s {
  int divider;
  int note_duration;
  int wholenote;
  int notes;
  int melody[250];
  int state = 0;
  int i;
  unsigned long prevM;
}; 

struct Buzzer_s buzzer;

void setup_Buzzer(struct Buzzer_s * ctx){
  ctx->divider = 0;
  ctx->note_duration = 0;
  ctx->wholenote = (60000 *4)/100;
  ctx->i = 0;
  ctx->prevM = 0;
  ctx->notes = sizeof(melody) / sizeof(melody[0]) /2;
}

void loop_Buzzer( struct Buzzer_s * ctx, struct mailbox_s *mb ) {

if( mb->state == FULL){
ctx->state = mb->value;
mb->state = EMPTY; 
}

if(ctx->state == 0){
    ctx->i=0;
    ledcDetachPin(17);
  return;

} 
   // for (int i = 0; i < ctx->notes*2; i = i+2){

    ctx->divider = melody[ctx->i+1];
    if(ctx->divider >0){
      ctx->note_duration = (ctx->wholenote)/ctx->divider;
    }
    else if(ctx->divider <0){
      ctx->note_duration = (ctx->wholenote) / abs(ctx->divider);
      ctx->note_duration *= 1.5;

    }
    
    tone(BUZZER_PIN, melody[ctx->i], ctx->note_duration*0.9, BUZZER_CHANNEL);
   // for(int j = 0; j < ctx->note_duration*0.9 ; j++){}
  
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
    
    return;

}

/* tache Led_s */
struct Led_s {
  int timer; 
  unsigned long period;
  unsigned long lum;
  int pin;
  int etat;
  int interrupt;
}; 

struct Led_s led;

void setup_Led( struct Led_s * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  ctx->interrupt = 0;
  ctx->lum = period;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void loop_Led( struct Led_s * ctx, struct mailbox_s *mb , struct mailbox_s *mb2) {

 // digitalWrite(LED_BUILTIN, ctx->etat);
  if (!waitFor(ctx->timer, ctx->period)) return;  

  if( ctx->interrupt == 1){
      ctx->etat = 0;
  }
  else
    ctx->etat = 1 - ctx->etat;  
  digitalWrite(LED_BUILTIN, ctx->etat);

/* led to oled for start/Stop msg */
  if( mb2->state == EMPTY){
    mb2->value = ctx->interrupt;
    mb2->state = FULL;
  }

  if( mb->state != FULL) return;                          // changement d'état
  //period inversement prop à la la lumiere
  ctx->period = ctx->lum * 1/((mb->value+1)/10);
  mb->state = EMPTY;   
  
}

void blink() {
  led.interrupt = !led.interrupt;
}

/* Oled_s */
struct Oled_s {
  int timer;             
  unsigned long period;      
  int val;
  int led_val;
};
struct Oled_s Oled;

void setup_Oled( struct Oled_s * ctx, int timer, unsigned long period, int val) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->val = val;
  ctx->led_val = 1;
  Serial.begin(9600);  
}

void loop_Oled(struct Oled_s *ctx, struct mailbox_s *mb , struct mailbox_s *mb2 ) {

   //check si boite pleine
   if( mb->state != FULL ) return;
   ctx->val = mb->value;

   mb->state = EMPTY;
  
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print((100-ctx->val));
  display.println("%");
  display.print("bp ");


/* led clign on bp to stop */
  if( ctx->led_val == 0)
    display.println("stop");
  else 
    display.println("shine");

  display.println("p: play");
  display.println("s: stop");


  /* led oled mb for start/stop msg */
  if( mb2->state == FULL) {
  ctx->led_val = mb2->value;
  mb2->state = EMPTY;
  }

  display.display(); 

}

/* Lum_s */
struct Lum_s {
  int timer;             
  unsigned long period;
};

struct Lum_s lum;

void setup_lum( struct Lum_s * ctx, int timer, unsigned long period) {
  ctx->timer = timer;
  ctx->period = period;
  Serial.begin(9600);                                     // initialisation du débit de la liaison série
}

void loop_lum(struct Lum_s *ctx, struct mailbox_s * mb, struct mailbox_s *mb2) {

  if( mb->state != EMPTY) return;
  //ne peut écrire que si la boite est vide
  mb->value = map(analogRead(36), 0, 4095, 0, 100 );

  //Serial.println(mb->value);
  
  //indique qu'elle a écrit
  mb->state = FULL;

  if( mb2->state != EMPTY) return;
  mb2->value = map(analogRead(36), 0, 4095, 0, 100 );
  mb2->state = FULL;
}
void setup() {

  Serial.begin(9600);
  Wire.begin(4, 15); // pins SDA , SCL
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

 setup_Led( &led , 0 , 1000000 , LED_BUILTIN);
 setup_Oled(&Oled, 1, 1000000, 0);
 setup_lum(&lum, 2, 500000 );
 setup_Buzzer( &buzzer);

/* setup BP */
  pinMode(23, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(23), blink, HIGH);
}
void loop() {

  loop_Led( &led , &mb_lum_led , &mb_led_Oled);
  loop_Oled( &Oled, &mb_lum_Oled , &mb_led_Oled);
  loop_lum(&lum , &mb_lum_Oled , &mb_lum_led );
  loop_Buzzer( &buzzer , &mb_serial_buzz );
  if( Serial.available())
    SerialEvent( &mb_serial_buzz);
}

void SerialEvent(struct mailbox_s * mb , struct mailbox_s * mb2){

  char lu = Serial.read();
  
  if( lu == 'p'){
    Serial.println("playing...");
    if( mb->state == EMPTY){
      mb->value = 1;
      mb->state = FULL;
    }

    if( mb2->state == EMPTY){
      mb2->value = 1;
      mb2->state = FULL;
    }

 
  }
  if( lu == 's'){
    Serial.println("stop...");
    if( mb->state == EMPTY){
      mb->value = 0;
      mb->state = FULL;
    }
    if( mb2->state == EMPTY){
      mb2->value = 0;
      mb2->state = FULL;
    }
  }
}


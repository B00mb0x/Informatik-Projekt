//Bibliotheken für das LCD einbinden
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  //LCD Adresse definieren 0x3F oder 0x27
//Timer Bibliothek einbinden
#include "TimerOne.h"
//nötig für den reset
#include <avr/wdt.h>

//Globale Variablen deklarieren
volatile int buttonState = LOW; //nötig für den Interrupt
int confirm = 1; //1 das Level wird eingestellt, 0 es wird gespielt
int loose = 0;//wird 1 wenn das Spiel verloren ist
int ende = 0;//für die serielle ausgabe
int richtung = 0;//die Richtung 0: Hoch 1:Rechts 2:Runter 3:Links
int level = 1;//bestimmt die Schnelligkeit der Schlange
int snakex[64];//x Position der Schlangenglieder
int buttonpress = 0;//Hilfsvariable damit jedes drücken des buttons nur einmal interpretiert wird
int snakey[64];//y Position der Schlangenglieder
int laenge = 2; //länge der Schlange
int apfel [2] = {1, 1}; //Position des Apfels
int shcp = 10; //shift register clock input
int ds = 12; //serial data input
int stcp = 11;//storage register clock input
int matrix[8] = {A0, A1, 4, 5, 6, 7, 8, 9}; //Ports an denen die LED Matrix direkt am Arduino angeschlossen ist
int bitmap[8][8] = { //leuchtende LEDs 0 LED aus 1 LED an
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

void setup() {
  //definieren der Pins für das Shiftregister als OUTPUT und LOW
  pinMode(shcp, OUTPUT);
  pinMode(ds, OUTPUT);
  pinMode(stcp, OUTPUT);
  digitalWrite(shcp, LOW);
  digitalWrite(ds, LOW);
  digitalWrite(stcp, LOW);
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);
  //definieren der Matrix pins als Output
  int i;
  for (i = 0; i < 8; i++) {
    pinMode(matrix[i], OUTPUT);
  }
  lcd.begin(16, 2);//bereitmachen des LCDs
  //Startposition der Snake
  snakey[0] = 3;
  snakex[0] = 3;
  snakey[1] = 3;
  snakex[1] = 4;
  Serial.begin(9600);//Serielle Ausgabe aktivieren
  attachInterrupt(1, buttonlinks, RISING); //Wenn der linke Button gedrückt wird wird die Funktin buttonlinks ausgeführt
  attachInterrupt(0, buttonrechts, RISING);//Wenn der rechte Button gedrückt wird wird die Funktin buttonrechts ausgeführt
  start();//das Startmenü in der funktion start wird ausgeführt
  Timer1.initialize(1000000 - (100000 * (level - 1)));//der Timer wird abhängig vom level eingestellt
  Timer1.attachInterrupt(updatesnake); //die Funktion updatesnake wird vom timer ausgeführt
}

void start() {
  //startet programm nach signal
  while (Serial.available() < 1) {

  }
  if (Serial.available() > 0) {
    level = Serial.read();
    if (level > 10) {
      level = level / 10;
    }
  }

  while (confirm) {//die Loop wird ausgeführt bis mit buttonrechts confirm auf 0 gesetzt wird
    //auf dem LCD wird das Level angezeigt
    lcd.clear();

    delay(100);
  }


}

void loop() {
  showLED();//die LEDs werden angezeigt
  if (loose == 1) {//wenn das Spiel verloren wurde wird die Länge der Schlange auf dem LCD angezeigt
    loose = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Laenge ");
    lcd.setCursor(9, 0);
    lcd.print(laenge);
    char str[32];
    sprintf(str, "%d", laenge);
    Serial.print(str);//schickt die Länge der Schlange an das C Programm
    ende = 1;
  }
  if (ende = 1) { //resetet arduino bei signal
    if (Serial.available() > 0) {
      wdt_enable(WDTO_15MS);
      while (1)
      {
      }
    }
  }
}

void buttonlinks()
{
  //wenn das Startmenü aktiv ist wird das level erhöht
  if (confirm == 1) {
    //    level++;
    //    //wenn das Level zu hoch wird, wird es auf 1 gesetzt
    //    if (level == 8) {
    //      level = 1;
    //    }
  }
  //wenn das Spiel aktiv ist bewegt sich die Schlange nach links
  else {
    if (buttonpress == 0) {//Fehlerhafte Signale werden ignoriert
      buttonpress = 1;
      richtung --;//Richtung geht nach links,links von Richtung 0 ist Richtung 3
      if (richtung == -1) {
        richtung = 3;
      }
    }
  }
}

void buttonrechts()
{
  //wenn das Startmenü aktiv ist wird das Spiel begonnen
  if (confirm == 1) {
    confirm = 0;
  }
  //wenn das Spiel aktiv ist bewegt sich die Schlange nach rechts
  else {
    if (buttonpress == 0) {//Fehlerhafte Signale werden ignoriert
      buttonpress = 1;
      richtung ++;//Richtung geht nach rechts,rechts von Richtung 3 ist Richtung 0
      if (richtung == 4) {
        richtung = 0;
      }
    }
  }
}

void matrixclear() {//stellt alle LEDs aus
  int i;
  for (i = 0; i < 8; i++) {
    int j, value;
    for (j = 0, value = 0; j < 8; j++) {
      bitmap [i][j] = 0;
    }
  }
}

void showLED() {//Zeigt die LEDs an
  int i;
  for (i = 0; i < 8; i++) {
    int j, value;
    for (j = 0, value = 0; j < 8; j++) {
      value = (value << 1) + bitmap[i][j];
    }
    write_to_line(i, value);
    delay(1);
  }
}

void verloren() {
  //stellt alle LEDs an
  int i;
  for (i = 0; i < 8; i++) {
    int j, value;
    for (j = 0, value = 0; j < 8; j++) {
      bitmap [i][j] = 1;
    }
  }
  loose = 1;//aktiviert loose für die LOOP
  Timer1.detachInterrupt();//stellt den Timer1 aus
}

void verlaengern() {
  laenge++;//verlängert die Schlange um 1
  //setzt das letzte Teil der Schlange auf den Vorletzten teil, sodass es nach einer Bewegung ganz hinten ist
  snakey[laenge] = snakey[laenge - 1];
  snakex[laenge] = snakex[laenge - 1];
}

void createapfel() {
  //erstellt zufällige position des Apfels
  randomSeed(millis());
  apfel[0] = random(8);
  apfel[1] = random(8);

  //prüft ob der Apfel auf der Schlange ist
  for (int i = 0 ; i < laenge; i++) {
    if (snakey[i] == apfel[0]) {
      if (snakex[i] == apfel[1]) {
        createapfel();//erstellt neuen Apfel wenn der Apfel auf der Schlange liegt
      }
    }
  }
  bitmap[apfel[0]][apfel[1]] = 1;//zeigt den Apfel auf der LED Matrix
}

void updatesnake() {
  matrixclear();//stellt alle LEDs aus
  bitmap[apfel[0]][apfel[1]] = 1; //zeichnet den Apfel

  if (buttonpress == 1) {//stellt buttonpress auf 0 damit die buttons wieder benutzt werden können
    buttonpress = 0;
  }

  if (snakey[0] == apfel[0] && snakex[0] == apfel[1]) {//wenn die schlange den apfel isst, wird sie länger und es wird ein neuer Apfel erstellt
    createapfel();
    verlaengern();
  }

  for (int i = (laenge - 1); i > 0; i--) {//alle teile der Schlange bis auf den Kopf werden nach vorne bewegt
    snakey[i] = snakey[i - 1];
    snakex[i] = snakex[i - 1];
  }
  switch (richtung) {//bestimmt in welche Richtung der Kopf sich bewegt
    case 0:
      snakey[0]++;
      break;
    case 1:
      snakex[0]++;
      break;
    case 2:
      snakey[0]--;
      break;
    case 3:
      snakex[0]--;
      break;
  }

  for (int i = 1; i < laenge; i++) {//wenn die Schlange sich selbst beißt ist das Spiel verloren
    if (snakey[i] == snakey[0]) {
      if (snakex[i] == snakex[0]) {
        verloren();
      }
    }
  }

  if (snakex[0] == -1 || snakey[0] == -1 || snakex[0] == 8 || snakey[0] == 8) {//wenn die Schlange das Spielfeld verlässt ist das Spiel verloren
    verloren();
  }
  else {//wenn das Spiel nicht verloren ist wird die Schlange gezeichnet
    for (int a = 0; a < laenge; a++) {
      bitmap[snakey[a]][snakex[a]] = 1;

    }
  }
}

void write_to_line(int line, int value) {
  write_to_matrix(0, 0, 0, 0, 0, 0, 0, 0);
  // inverted mode, because we use it
  // for the cathode.
  write_to_shifter(255 - value);
  write_to_matrix(line == 0, line == 1,
                  line == 2, line == 3, line == 4,
                  line == 5, line == 6, line == 7);
}

void write_to_matrix(int v1, int v2,
                     int v3, int v4, int v5,
                     int v6, int v7, int v8) {
  digitalWrite(matrix[0], v1);
  digitalWrite(matrix[1], v2);
  digitalWrite(matrix[2], v3);
  digitalWrite(matrix[3], v4);
  digitalWrite(matrix[4], v5);
  digitalWrite(matrix[5], v6);
  digitalWrite(matrix[6], v7);
  digitalWrite(matrix[7], v8);
}

void write_to_shifter(int value) {
  for (int i = 7; i >= 0; --i) {
    digitalWrite(shcp, LOW); //clock low
    int b = value & (1 << i);
    digitalWrite(ds, b); // set the bit
    digitalWrite(shcp, HIGH);//shift on high
  }
  digitalWrite(stcp, HIGH); // show on high
  digitalWrite(stcp, LOW); // clock low
}


/****************************************************************************
 * Project: Bracelet de chute - Projet Etat de l'art
 * Authors: Corentin Antony, Mathieu Grossard, Quentin Schultz, Theo Phil Lange
 ****************************************************************************/
#include "U8glib.h"           //Librairie et définition LCD SH1106 128X64
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"          // MPU6050
#include <SoftwareSerial.h>   // GPS et GSM


/****************************************************************************
 * PIN DEFINITIONS
 ****************************************************************************/
#define PIN_BUZZER 12
#define PIN_CAPTEUR_VIBRATION 11
#define PIN_BUTTON_TVB 2
#define PIN_BUTTON_SOS 3
#define PIN_GPS_RX 5
#define PIN_GPS_TX 4
#define PIN_GSM_RX 6
#define PIN_GSM_TX 7


/****************************************************************************
 * GLOBAL VARIABLE AND OBJECT DEFINITIONS
 ****************************************************************************/
MPU6050 AccelGyro;                                      // Objet du MPU6050
U8GLIB_SH1106_128X64 LCD(U8G_I2C_OPT_NONE);             // I2C / TWI
SoftwareSerial SoftSerial_GPS(PIN_GPS_RX, PIN_GPS_TX);  // objet Serial GPS
SoftwareSerial SoftSerial_GSM(PIN_GSM_RX, PIN_GSM_TX);  // objet Serial GSM

unsigned char buffer[64];                               // buffer array for data receive over serial port
int count = 0;                                          // counter for buffer array

float AccelGyro_shock_float = 0.0;
volatile bool bool_ButtonTVB = 0;
volatile bool bool_ButtonSOS = 0;


/****************************************************************************
 * FUNCTIONS
 ****************************************************************************/
void LCD_initialize()
{
  /* La fonction permet initiaaliser l'ecran LCD */

  // assign default color value
  if ( LCD.getMode() == U8G_MODE_R3G3B2 ) {
    LCD.setColorIndex(255);                           // white
  }
  else if ( LCD.getMode() == U8G_MODE_GRAY2BIT ) {
    LCD.setColorIndex(3);                             // max intensity
  }
  else if ( LCD.getMode() == U8G_MODE_BW ) {
    LCD.setColorIndex(1);                             // pixel on
  }
  else if ( LCD.getMode() == U8G_MODE_HICOLOR ) {
    LCD.setHiColorByRGB(255,255,255);
  }
}

void LCD_draw(float shock=0.0, int choice=0, int decompte=0)
{
  /* La fonction permet d'afficher differentes ecrans predefiné sur le LCD */

  /* commandes pour l'affichage LCD */
  /*
   * u8g.setFont(u8g_font_unifont);
   * u8g.setFont(u8g_font_osb21);
   * u8g.drawStr( 0, 22, "Hello World!");
   */

  LCD.setFont(u8g_font_unifont);  // choisir type des caractéres

  switch(choice){
    case 0: {
      // L'affichage pour une utilisation debug
      LCD.setPrintPos(0,20);          // curseur a la position (x,y)
      LCD.print("Bracelet de");       // texte affiché premier ligne sur l'ecran
      LCD.setPrintPos(0,40);
      LCD.print("chute");
      LCD.setPrintPos(80,60);
      LCD.print(shock);               // affiche shock actuelle
    }
    break;
    case 1: {
      LCD.setPrintPos(0,40);
      LCD.print("Envoyer message");
      LCD.setPrintPos(50,60);
      LCD.print("SOS!");
    }
    break;
    case 2: {                         // l'ecran teste pour une message tout va bien
      LCD.setPrintPos(0,40);
      LCD.print("Envoyer message");
      LCD.setPrintPos(0,60);
      LCD.print(" tout va bien!");
    }
    break;
    case 3: {
      LCD.setPrintPos(0,40);
      LCD.print("Message envoye!");
    }
    break;
    case 4: {
      // L'affichage en mode normal
      LCD.setPrintPos(35,20);
      LCD.print("BRACELET");
      LCD.setPrintPos(60,40);
      LCD.print("DE");
      LCD.setPrintPos(47,60);
      LCD.print("CHUTE");
    }
    break;
    case 5: {
      LCD.setPrintPos(20,20);
      LCD.print("Est-ce que");
      LCD.setPrintPos(10,40);
      LCD.print("tout va bien?");
      LCD.setPrintPos(60,60);
      LCD.print(decompte);
      LCD.print("s");
    }
    break;
    case 6: {
      LCD.setPrintPos(40,40);
      LCD.print("OK! :)");
    }
    break;
    case 7: {
      LCD.setPrintPos(20,40);
      LCD.print("Bouton SOS");
      LCD.setPrintPos(40,60);
      LCD.print("presse");
    }
    break;
    default: {
      LCD.setPrintPos(60,40);
      LCD.print("ERREUR");
    }
  }
}

void MPU_read()
{
  /* Lire les données brutes acceleration / gyro */
  int16_t ax, ay, az;                 // mesures brutes
  int16_t gx, gy, gz;                 // pour le gyroscope
  //int16_t ax_moy, ay_moy, az_moy;   // moyennes
  //int16_t gx_moy, gy_moy, gzv;
  AccelGyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  //ax_moy = ax;

  /* Autre méthodes pour cela */
  /*
   * //accelgyro.getAcceleration(&ax, &ay, &az);
   * //accelgyro.getRotation(&gx, &gy, &gz);
   */

  /* Possibilité debug pour les valeurs de l'accyleroscope (accel/gyro x/y/z) */
  /*
   * Serial.print("a/g:\t");
   * Serial.print(ax); Serial.print("\t");
   * Serial.print(ay); Serial.print("\t");
   * Serial.print(az); Serial.print("\t");
   * Serial.print(gx); Serial.print("\t");
   * Serial.print(gy); Serial.print("\t");
   * Serial.print(gz); Serial.print("\t");
   * // x > 0 si bascule en avant (tanguage)
   * // y > 0 si roulis du côté gauche
   * // z > 0 si orienté en haut
   */

  //== module détecteur de chocs ou d'accélérations (sans 1g)
  AccelGyro_shock_float = (float)ax*ax + (float)ay*ay + (float)az*az;
  AccelGyro_shock_float = abs(sqrt(AccelGyro_shock_float)/1000-16);   //retirer 1g et absolu
  Serial.print(AccelGyro_shock_float); Serial.println(";");

  /* Possibilité debug la valeur de chock en etoiles */
  /*
   * String etoiles = "*****************************";
   * String nb_etoiles = etoiles.substring(0,(int)AccelGyro_shock_float);
   * Serial.print(AccelGyro_shock_float);
   * Serial.print(" ");
   * Serial.println(nb_etoiles);  //module
   */

  delay (100);
}

void clearBufferArray()
{
  /* fonction pour effacer le buffer array */
  for (int i=0; i<count;i++)
  {
    buffer[i]=NULL;
  }                      // clear all index of array with command NULL
}

String GPS_get()
{
  /* récuperer des coordinates GPS avec une masque string */
  bool done = 0;
  char currentchar = '.';
  while(done == 0){
    if(SoftSerial_GPS.available()){
      currentchar = SoftSerial_GPS.read();
      if(currentchar=='$'){
        count = 0;
        buffer[count++] = '$';
      }
      if(currentchar=='*'){
        if(buffer[1]=='G' && buffer[2]=='N' && buffer[3]=='G' && buffer[4]=='G' && buffer[5]=='A'){
          Serial.write(buffer,count);
          Serial.println(" ");
          String GPS = buffer;
          Serial.print(GPS);
          return GPS;
          done = 1;
        }
      }
      if(currentchar!='*' && currentchar!='$'){
        buffer[count++] = currentchar;
      }
    }
  }
}

void GSM_sendSMS(int choix=0) // ajouter parametre pour donner coordinates GPS
{
  /* Cette fonction permet d'envoyer une SMS. Avec le premier paramètre on peut choisir le typ du SMS, il y a:
   *   - choix = 0: Je suis tombé avec coordinates GPS
   *   - choix = 1: Tout va bien
   *   - choix = 2: SOS
   */
  Serial.println("Get GPS...");
  String GPS = GPS_get();
  Serial.println("cordonnées GPS récuperé...");
  Serial.println("Sending SMS...");                    // Show this message on serial monitor
  String message;
  switch(choix){
    case 0: { message = "Je suis tombe *COORDINATES GPS* "; message += GPS; Serial.println(message); }
    break;
    case 1: message = "Tout va bien!";                // option pour tester
    break;
    case 2: { message = "SOS! *COORDINATES GPS* "; message += GPS; Serial.println(message); }
    break;
    default: Serial.println("ERROR - Choisir type de SMS");
  }
  SoftSerial_GSM.print("AT+CMGF=1\r");                 // Set the module to SMS mode
  delay(100);
  SoftSerial_GSM.print("AT+CMGS=\"+33769542703\"\r");  // numero de telephone du destinaire
  delay(500);
  SoftSerial_GSM.print(message);                       // message a l'envoyer pour le destinaire
  delay(500);
  SoftSerial_GSM.print((char)26);
  delay(500);
  SoftSerial_GSM.println();
  Serial.println("Text Sent.");
  delay(500);
}



/****************************************************************************
 * SETUP
 ****************************************************************************/
void setup(){
  Wire.begin();                 // Initialisation bus I2C
  SoftSerial_GSM.begin(9600);   // Initialisation communication GSM
  SoftSerial_GPS.begin(9600);   // Initialisation communication GPS
  Serial.begin(9600);           // Initialisation communication PC sur console serie
  Serial.println("---------------------SETUP---------------------");
  Serial.println(AccelGyro.testConnection() ? "MPU6050 connection reussie" : "MPU6050 connection echec");
  Serial.println("Initialisation bus serie fini");

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_CAPTEUR_VIBRATION, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_TVB), ISR_toutVaBien, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_SOS), ISR_SOS, FALLING);
  Serial.println("Definition des PINs fini..");

  AccelGyro.initialize();

  Serial.println("Initialisation d'Accelometre + Gyroscope fini..");

  LCD_initialize();
  Serial.println("Initialisation de LCD fini");

  Serial.println("---------------------LOOP----------------------");
}


/****************************************************************************
 * LOOOP
 ****************************************************************************/
void loop(){
  MPU_read();   // Lit les valeurs de l'acceleromètre

  //bool CapVib_bool = digitalRead(PIN_CAPTEUR_VIBRATION); // CAPTEUR VIBRATION
  if(AccelGyro_shock_float >= 23){ // definition l'intensité d'une chock qui est une chute (ici 23)
    /* Si un choc se produit, on active le buzzer */
    digitalWrite(PIN_BUZZER,HIGH);
    bool_ButtonTVB = 0;             // on traite juste le bouton TVB si nous somme apres une chute
                                    // ca eveite que nous avons une chute qui n'est pas traité

    for(int decompte = 10; decompte>=0; decompte--){
      // Apres une chute la person a dix secondes pour indiquer avec le bouton que tout va bien
      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,5,decompte);
      }while( LCD.nextPage() );

      if(bool_ButtonTVB){ break; }
      delay(1000);
    }
    digitalWrite(PIN_BUZZER,LOW);

    if(bool_ButtonTVB){ // si la person indique que tout va bien on envoie pas une message
      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,6);
      }while( LCD.nextPage() );
      delay(300);
      for(int i=0; i<2; i++){
        digitalWrite(PIN_BUZZER,HIGH);
        delay(300);
        digitalWrite(PIN_BUZZER,LOW);
        delay(300);
      }

      delay(2000);
      bool_ButtonTVB = 0;
    }
    else{ // si la person n'indique pas que tout va bien on envoie une message SOS
      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,1);
      }while( LCD.nextPage() );

      GSM_sendSMS(0);

      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,3);
      }while( LCD.nextPage() );

      delay(2000);
      bool_ButtonTVB = 0;
    }
  }
  else{
    LCD.firstPage();
    do{
      LCD_draw(AccelGyro_shock_float,4);
    }while( LCD.nextPage() );
  }

/* Routine pour tester le bouton Tout va Bien */
/*
 * if(bool_ButtonTVB){
 *   LCD.firstPage();
 *   do{
 *     LCD_draw(AccelGyro_shock_float,2);
 *   }while( LCD.nextPage() );
 *
 *   for(int i=0; i<2; i++){
 *     digitalWrite(PIN_BUZZER,HIGH);
 *     delay(300);
 *     digitalWrite(PIN_BUZZER,LOW);
 *     delay(300);
 *   }
 *   GSM_sendSMS(1);
 *
 *   LCD.firstPage();
 *   do{
 *     LCD_draw(AccelGyro_shock_float,3);
 *   }while( LCD.nextPage() );
 *
 *   delay(2000);
 *   bool_ButtonTVB = LOW;
 * }
 */

  if(bool_ButtonSOS){
    /* Envoie la message "SOS!" si la bouton a droit est presé */
    LCD.firstPage();
    do{
      LCD_draw(AccelGyro_shock_float,7);
    }while( LCD.nextPage() );

    for(int i=0; i<10; i++){          // son indiquation que le bouton SOS etait pressé
      digitalWrite(PIN_BUZZER,HIGH);
      delay(50);
      digitalWrite(PIN_BUZZER,LOW);
      delay(50);
    }
    delay(500);

    bool_ButtonTVB = 0;             // on traite juste le bouton TVB si nous somme apres une chute
                                    // ca eveite que nous avons une chute qui n'est pas traité

    for(int decompte = 10; decompte>=0; decompte--){
      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,5,decompte);
      }while( LCD.nextPage() );

      if(bool_ButtonTVB){ break; }
      delay(1000);
    }

    if(bool_ButtonTVB){
      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,6);
      }while( LCD.nextPage() );
      delay(300);
      for(int i=0; i<2; i++){
        digitalWrite(PIN_BUZZER,HIGH);
        delay(300);
        digitalWrite(PIN_BUZZER,LOW);
        delay(300);
      }
      delay(2000);
    }
    else{ // si la person n'indique pas que tout va bien on envoie une message SOS
      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,1);
      }while( LCD.nextPage() );

      GSM_sendSMS(2);

      LCD.firstPage();
      do{
        LCD_draw(AccelGyro_shock_float,3);
      }while( LCD.nextPage() );

      delay(2000);
    }
    bool_ButtonTVB = 0;
    bool_ButtonSOS = 0;
  }
}


/****************************************************************************
 * INTERRUPT ROUTINES
 ****************************************************************************/
void ISR_toutVaBien(){
  bool_ButtonTVB = HIGH;
}

void ISR_SOS(){
  bool_ButtonSOS = HIGH;
}


/****************************************************************************
 * END
 ****************************************************************************/

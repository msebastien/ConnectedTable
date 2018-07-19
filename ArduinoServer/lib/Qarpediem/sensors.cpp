#include <DHT.h>
#include <ChainableLED.h>
#include <Qarpediem.h>
#include <Multiplexer.h>
#include <SensorServer.h>

#include "sensors.h"

extern Mux8 mux_com;
extern DHT dht;
extern short int count_IR[6];
extern int mouvement;
extern ChainableLED status_led;
extern bool requestReceived;

// DISTANCE
unsigned int distance[6] = {0, 0, 0, 0, 0, 0};

// IR SENSOR
// Paramétre de calibration des capteurs IR
float calibration2_IR[6] = {1.26, 1.34, 1.34, 1.36, 1.36, 1.34};

extern char tableStatus[6];

// Data
char tableData[9];
char tableData_save[9];
int rawTemperatureIR[6]; // Millivolts
int rawDistance[6]; // Millivolts

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                           TEMPERATURE AMBIANTE/HUMIDITE
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  capteur_temp() & capteur_hum(): Capteur température et humidité 101020088
   â€¢Utilise la bibliothÃ©que DHT pour lire la température (°C) et l'humidity (%) retourner par le capteur
     â€¢Comme les data sont transmise en valeur entiere , les fonction retourne de valeur entiÃ¨re
     â€¢si le capteur n'est pas connectÃ©, alors on retourne la valeur 0xFF moins la valeur d'ajout effectuer à la constitution de la trame
*/
int capteur_temp()
{
    mux_com.select_output(0);

    float t = dht.readTemperature();
    int temperature = t;
    if ( t - temperature >= 0.5) temperature++; //arrondi

    return temperature;
}


unsigned int capteur_hum()
{
    mux_com.select_output(0);

    float h = dht.readHumidity();
    unsigned int humidity = h;
    if (h - humidity >= 0.5) humidity++; //arrondi

    return humidity;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                        CAPTEUR DE TEMPERATURE INFRAROUGE
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  capteur_IR(): capteur de TÂ° IR ZTP-115M

  La data concernant les capteurs de temperature IR est consituer de 1 octet, se sont les bits qui indiqueront si oui ou non, les capteurs indique une prÃ©sence, ainsi pour le capteur numÃ©ro 1 ( adresser grÃ¢ce a la fonction Mux)
    il correspondra au bit de poids faible ( pow(2,0) =1), le capteur 2, le bit suivant ( pow(2,1)=2) ect... jusqu'Ã  la lecture des 6 capteurs maximum ( le programme peut etre converti vers 8 )

    à noter que la fonction pow est instable, et par exemple, pow(2,2) = 4.00 , mais si on le converti vers un entier on obtient 3, alors comme on souhaite un entier au final on ajouter 0.5 à cette puissance

    Il est important de calibré correctement les capteurs grâce au tableau en variable globale : calibration_IR

    les commentaire dans cette focntion permettent d'afficher la valuer analogique que retourne chaque capteur IR, elle permet de calibré manuellemnt les capteurs
*/
unsigned int capteur_IR()
{
    unsigned int Temp_objet = 0;
    int nbr = 1;
    int Capteur_IR = 0;
    int i = 0;
    unsigned int t = capteur_temp();

    while (nbr < (Qarpediem::NB_PLACES + 1)) {
      mux_com.select_output(nbr); //dirige l'entrée vers le capteur nbr

      for (i = 0; i < 10; i++) Capteur_IR += analogRead(Qarpediem::C_IR_PIN); // Lis la valeur renvoyée par le capteur
      Capteur_IR /= 10;
      if(!isnan(Capteur_IR) || Capteur_IR > 0) // If it's a finite number
      {
          if (Capteur_IR >= 213 * (0.014 * t + calibration2_IR[nbr - 1]) && count_IR[nbr - 1] < 3 ) count_IR[nbr - 1]++;

          else if (count_IR[nbr - 1] > -3) count_IR[nbr - 1]--;

          if (count_IR[nbr - 1] >= 0)Temp_objet += (pow(2, nbr - 1) + 0.5);
      }

      nbr++;
    }

    return Temp_objet;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                         CAPTEUR DE DISTANCE
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  capteur_distance(unsigned int distance[]) : capteur de distance 101020042
    Le capteur a une prÃ©cision maximale de 80 cm, une courbe d'Ã©quation a Ã©tait Ã©tabli pour obtenir une relation entre la distance mesurÃ© et sa Tenssion en sortie (25.291 (U^-1.139))
    Le systÃ¨me pouvant gÃ©rer jusqu'Ã  6 places, ce capteur reserve 6 octets dans la Trame, 1 octets par capteur, ainsi, pour les capteurs non connectÃ©s, on met cet valeur Ã  255 par dÃ©faut (254 a cause de l'opÃ©ration effectuer dans la trame)

    Puis on  indique quel capteur on veut mesurer grÃ¢ce a la fonction Mux, et on fait la moyenne d'un certain nombre de mesures
    Comme le capteur a une prÃ©cision maximum de 80 cm, si la valeur retourner est supÃ©rieur Ã  80 cm, alors on peut concidÃ©rer que le capteur n'a pas d'objet devant lui ( ni chaise ni personne)
    C'est pourquoi lorsque le capteur retourne une valeur supÃ©rieur Ã  150 cm, on est largement au dessus de la limite du capteur, pour Ã©viter ainsi un conflit avec le "0xFF", ou mÃªme  de dÃ©passer la valeur d'un octet, on dÃ©fini 150 comme valeur maximum

    enfin on retourne le tableau ainsi constituer, ( distance[0] correspond au capteur 1, distance[1] au capteur 2, ect...)
*/
void capteur_distance()
{
  double voltage = 0, distance_capteur = 0, total = 0;
  short int nbr = 1;
  int i = 0, j = 0;

  for (i = 0; i < Qarpediem::NB_PLACES; i++) {
    distance[i] = 254; //initialisation par default comme non connectÃ©
  }
  while (nbr < (Qarpediem::NB_PLACES + 1)) {
    total = 0;

    mux_com.select_output(nbr); //dirige l'entrée vers le capteur nbr

    for (j = 0; j < 50; j++) {
      voltage = analogRead(Qarpediem::C_DIST_PIN) * 5.0 / 1023.0; //conversion de la valeur lue en Volt
      distance_capteur = 25.291 * (pow(voltage, -1.139)); //conversion des Volts vers une distance
      total += distance_capteur; //somme des 50 distances mesurées
    }
    total = total / 50; //moyenne des distances
    if (total > 150) total = 150;

    if(!isnan(total) || total > 0) // If it's a finite number
    {
        distance[nbr - 1] = total;
    }

    nbr++;
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                        DONNEES BRUTES (DISTANCE / TEMPERATURE IR)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void getRawTempIR()
{
    int nbr = 1;
    float voltageTempIR = 0;
    int i = 0;

    while (nbr < (Qarpediem::NB_PLACES + 1)) {
      mux_com.select_output(nbr); //dirige l'entrée vers le capteur nbr

      for (i = 0; i < 10; i++) voltageTempIR += analogRead(Qarpediem::C_IR_PIN) * 5000.0 / 1023.0; // Millivolts
      voltageTempIR /= 10.0;

      rawTemperatureIR[nbr - 1] = (int)voltageTempIR;

      nbr++;
    }

}

void getRawDistance()
{
  float voltage = 0, total = 0;
  short int nbr = 1;
  int i;

  while (nbr < (Qarpediem::NB_PLACES + 1)) {
    total = 0;

    mux_com.select_output(nbr); //dirige l'entrée vers le capteur nbr

    for (i = 0; i < 50; i++) {
      voltage = analogRead(Qarpediem::C_DIST_PIN) * 5000.0 / 1023.0; //conversion de la valeur lue en Millivolt
      total += voltage; //somme des 50 distances mesurées
    }
    total /= 50.0; //moyenne des tensions

    rawDistance[nbr - 1] = (int)total;

    nbr++;
  }

}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                         CAPTEUR DE MOUVEMENT
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  capteur_mouvement() : detecteur de mouvement 101020060
    DÃ©fini sur un attachInterupt dans le setup() , movement sert Ã  changer la variable "mouvement" a chaque que le capteur de mouvement change d'Ã©tat ( 0 vers 1 ou 1 vers 0)
*/
void capteur_mouvement()
{
    mouvement = digitalRead(Qarpediem::C_MOUVEMENT_PIN);
}

//capteur de son (desactive)
unsigned int capteur_son() {
  /*if (Capteurs[4]) {
    //a completer
    return 1;
  }
  else*/ return 254;
}

/*
    data_mvmt_Vibe(unsigned int Vibe)

  Permet de constituer la data contenant l'information sur la vibration, et le mouvement ( les deux bit de poids faible sont le mouvement, , les 2 suivantet la vibration, les 4 bits supérieur sont disponible en cas de besoin d'ajoute de capteurs
*/
unsigned int data_mvt() {
  unsigned int data = 0;

  if (mouvement == 255) data += 1;
  else if (mouvement == 0)data += 2;
  else data += 3;

  /*if (Vibe == 255) data += 4;
  else if (Vibe == 0)data += 8;
  else data += 12;*/

  return data;
}

void createDataTable(unsigned int obj_chaud, unsigned int data_mvmt, unsigned int bruit) {
  //initialisation des valeur reÃ§u au format char (modifiable au besoin)
  int i;
  char chaud = obj_chaud + 1, br = bruit + 1, data_movement = data_mvmt + 1;
  char dist[6] = {distance[0] + 1, distance[1] + 1, distance[2] + 1, distance[3] + 1, distance[4] + 1, distance[5] + 1};

  //CrÃ©ation trame secondaire pour faciliter le remplissage(Ã  Traver une boucle) de la Trame mit en variable globale
  char Trame2[9] = {dist[0], dist[1], dist[2], dist[3], dist[4], dist[5], chaud, data_movement, br};

  for (i = 0; i < 9; i++) tableData[i] = Trame2[i];

} // Fin de createDataTable()

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                       EVENEMENT
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  boolean event ()
    La fonction permet de detecter si un changement important a eu lieu au niveau des capteurs, qui permetra de determinÃ© si il faut communiquer la trame ou non .
    retourne 1 si un Ã©vÃ©nement important a eu lieu, 0 si non

    data_save[] correspond a la Trame prÃ©cÃ©dente
    Si data_save[0] = 0, c'est que nous somme a l'intialisation, donc qu'il faut envoyer une premiÃ¨re data.

*/
boolean event() {

  if (tableData_save[0] == 0) { //premiÃ¨re trame
    strcpy(tableData_save, tableData);
    return 1;
  }

  // DISTANCE
  int i;
  for (i = 0; i < 5; i++) {

    if ((tableData_save[i] >= 80) && (tableData_save[i] != 254)) { //distance (n-1) superieur Ã  80 cm et capteur connectÃ©
      if ((tableData[i] <= 80) && (tableData[i] <= tableData_save[i] - 15)) { //distance (n) inferieur Ã  80 cm et variation notable (15 cm)
        strcpy(tableData_save, tableData);
        return 1;
      }

    }
    else if ((tableData[i] <= (tableData_save[i] - 15)) || (tableData[i] >= (tableData_save[i] + 15))) { //variation notable (+ ou - 15 cm)
      strcpy(tableData_save, tableData);
      return 1;
    }

  }

  // IR & MOUVEMENT
  if (tableData[6] != tableData_save[6]) { //octet correspondant au capteur IR
    strcpy(tableData_save, tableData);

    return 1;
  }
  else if (tableData[7] % 2 != tableData_save[7] % 2) { //changement d'etat de la detection de mouvement
    strcpy(tableData_save, tableData);
    return 1;
  }
  else return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                         COULEURS DES LEDs
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
   Cette partie du programme n'est utile que pour la présentation des table, elle comprend l'affichage de place à l'aide de LED, ainsi qu'un écran LCD
   Cette section se base donc sur un mode "Table" à 6 chaises
*/

/*void color(unsigned int distance[], unsigned int data_IR)
   cette décide de quel couleur doit prendre la diode en fonction des distance et de la chaleur donnée par les capteurs , et lui applique directement
   la fonction doit être placer après avoir vérifier la fonction event()
*/
void color(unsigned int data_IR) {
  int bit_IR = 0;
  for (int i = 0; i < Qarpediem::NB_PLACES; i++) {
    bit_IR = pow(2, i) + 0.5;


    if (distance[i] > 70) status_led.setColorRGB(i, 0, 0, 255); //Blue
    else if (data_IR & bit_IR) status_led.setColorRGB(i, 255, 0, 0); // Red
    else status_led.setColorRGB(i, 50, 255, 0); // Green
  }
} // Fin de color()

void get_table_status(unsigned int data_IR) {
  int bit_IR = 0;
  for (int i = 0; i < Qarpediem::NB_PLACES; i++) {
    bit_IR = pow(2, i) + 0.5;


    if (distance[i] > 70) tableStatus[i] = '0'; // No chair
    else if (data_IR & bit_IR) tableStatus[i] = '2'; // Presence of a person
    else tableStatus[i] = '1'; // Chair available
  }
} // Fin de status()

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                     ROUTINE ("MAIN FUNCTION executed in loop()")
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*void routine()
   Correspond à la routine du programme, la fonction est éxécuter en boucle .
   Elle consiste à lire les valeurs des capteurs, vérifier les message reçu, vérifier si il faut envoyer la nouvelle Trame, et si oui, la transmettre.
*/
void sensors_routine() {
    Serial.println(F("==Sensors routine=="));

        //Initialisation des valeurs de chaque capteurs
        capteur_distance();
        unsigned int obj_chaud = capteur_IR();
        unsigned int bruit = capteur_son();
        unsigned int data_mvmt = data_mvt();
        getRawDistance();
        getRawTempIR();

        //Création de la trame en fonction des nouvelles valeur des capteurs
        createDataTable(obj_chaud, data_mvmt, bruit);

        // Verification d'un éventuelle changement important par rapport à la trame précédente
        bool getEvent = event();

        // Si un évènement survient
        if (getEvent) {
          Serial.println(F("Got an event"));
          color(obj_chaud);
          get_table_status(obj_chaud);
        }

        Serial.println();
        for(int i =0; i < 9; i++){
            Serial.print(tableData[i]);
        }
        Serial.println();

} // Fin de routine()

#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <Arduino.h>

int capteur_temp(void);
unsigned int capteur_hum(void);
unsigned int capteur_IR(void);
void capteur_distance();
void capteur_mouvement(void);
void getRawTempIR(void);
void getRawDistance(void);
unsigned int data_mvt();

void createDataTable(unsigned int obj_chaud, unsigned int data_mvmt, unsigned int bruit);
boolean event(void);
void color(unsigned int data_IR);
void get_table_status(unsigned int data_IR);
void sensors_routine();


#endif

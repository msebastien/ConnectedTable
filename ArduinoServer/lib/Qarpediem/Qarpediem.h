#ifndef __QARPEDIEM_PIN__
#define __QARPEDIEM_PIN__

#include <Arduino.h>
#include "SensorServer.h"

namespace Qarpediem {
  int const MUX_PIN1 = 4;
	int const MUX_PIN2 = 5;
	int const MUX_PIN3 = 6;

    // CAPTEURS
    //pin des capteurs

    // PIN A0
    int const C_TEMPHUM_PIN = A0; // Correct (A0)

    // PIN A1
    int const C_SOUND_PIN = A1;
    int const C_IR_PIN = A1;
    int const C_DIST_PIN = A1; 


    int const C_MOUVEMENT_PIN = 2;
    int const C_INTERRUPT_PIN = 0;

	   // LED
	   int const LED_CLOCK_PIN = 7;
	   int const LED_DATA_PIN = 8;

     // Nombre de places/chaises
     int const NB_PLACES = 1; // Par défaut, une table comporte 6 chaises

     // SERIAL COMMUNICATION
     int const DATA_SENT_SIGNAL_PIN = 16;
     int const DATA_RECEIVED_SIGNAL_PIN = 17;
    //int const RX_PIN = 11;
	  //int const TX_PIN = 10;
	  HardwareSerial* const SERIAL_COM = &Serial1;
};

#endif // __QARPEDIEM_PIN__

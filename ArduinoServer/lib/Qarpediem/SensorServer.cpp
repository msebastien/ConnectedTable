#include <Arduino.h>
#include <DHT.h>
#include <Multiplexer.h>
#include <ChainableLED.h>
#include <Digital_Light_TSL2561.h>
#include <SensorAverage.h>
#include <DuplexBase.h>
#include <stdlib.h>

#include "SensorServer.h"
#include "Qarpediem.h"
#include "sensors.h"

using namespace Qarpediem;
using namespace InterQarpe;

//-----------------------------------------------------------------------------------------------------
// GLOBAL DEFINITIONS
//-----------------------------------------------------------------------------------------------------

Mux8 mux_com(Qarpediem::MUX_PIN1, Qarpediem::MUX_PIN2, Qarpediem::MUX_PIN3);

DHT dht(Qarpediem::C_TEMPHUM_PIN, DHT22);

SensorAverage motion_sensor;
SensorAverage sound_sensor;

ChainableLED status_led(Qarpediem::LED_CLOCK_PIN, Qarpediem::LED_DATA_PIN, Qarpediem::NB_PLACES);

//DETECTION DE MOUVEMENT
int mouvement = 0; //est modifie par un attachInterupt sur le capteur de mouvement
bool requestReceived = false;

// Status des places
char tableStatus[6];
char placeInexistante = '3';
float nonDisponible = (int)placeInexistante - 48;

//DETECTION IR
short int count_IR[6]; //compte le nombre de fois où le capteur a retourner une valeur d'un coté ou de l'autre du seuil ( varie de -3 à +3)

// DISTANCE
extern unsigned int distance[6];

extern char tableData_save[9];
extern char tableData[9];

//------------------------------------------------------------------------------------------------------

// SensorServer constructor
SensorServer::SensorServer(HardwareSerial *serialPort)
    : serial_port(serialPort), last_poll_average(0) {}

void SensorServer::init(void)
{
    serial_port->begin(230400);

    // Initalisation de la LED
    status_led.init();
    delay(2000);
    // Init Temperature / Humidity sensor
    dht.begin();

    // Initialise le capteur de mouvement sur un attachInterrupt, la variable attribué au capteur de mouvement changera automatiquement
    pinMode(C_MOUVEMENT_PIN, INPUT);
    attachInterrupt(C_INTERRUPT_PIN, capteur_mouvement, CHANGE);

    tableData_save[0] = 0;

    // Init count_IR
    for (int i = 0; i < 6; i++)
    {
        count_IR[i] = 0;
    }

    // Init Table Status
    for (int i = 0; i < 6; i++)
    {
        tableStatus[i] = '\0';
    }

    for (int j = 0; j < 6; j++)
    {
        tableStatus[j] = '3';
    }

    pinMode(Qarpediem::MUX_PIN1, OUTPUT);
    pinMode(Qarpediem::MUX_PIN2, OUTPUT);
    pinMode(Qarpediem::MUX_PIN3, OUTPUT);

    pinMode(Qarpediem::C_DIST_PIN, INPUT);
    pinMode(Qarpediem::C_IR_PIN, INPUT);

    Serial.flush();
}

void SensorServer::routine(void)
{
    DuplexBase::routine();

    sensors_routine();

    float sampling_period = motion_sensor.get_sampling_period() * 1000;
    uint32_t now = now_ms();

    // MOTION & SOUND
    if (now - last_poll_average >= (uint32_t)sampling_period)
    {
        last_poll_average = now;

        motion_sensor.push_value(digitalRead(Qarpediem::C_MOUVEMENT_PIN));

        mux_com.select_output(0);
        sound_sensor.push_value(analogRead(Qarpediem::C_SOUND_PIN));
    }

    if (is_connected())
    {
        Serial.println(F("CONNECTED"));
    }
    else
    {
        Serial.println(F("DISCONNECTED"));
    }
}

int SensorServer::write_bytes(uint8_t *buffer, size_t buffer_length)
{
    return serial_port->write(buffer, buffer_length);
};

int SensorServer::read_bytes(uint8_t *buffer, size_t buffer_length)
{
    return serial_port->readBytes(buffer, buffer_length);
}

size_t SensorServer::bytes_available()
{
    return serial_port->available();
}

uint32_t SensorServer::now_ms()
{
    return millis();
}

void SensorServer::on_sensor_distance(char id)
{
    requestReceived = true;
    float dist1, dist2, dist3, dist4, dist5, dist6;

    switch (id)
    {
    case '1':
        dist1 = (int)distance[0];
        requestReceived = false;
        Serial.print("distance1:");
        Serial.println(dist1);
        send_response_ok(dist1);
        break;

    case '2':
        dist2 = (int)distance[1];
        requestReceived = false;
        Serial.print("distance2:");
        Serial.println(dist2);
        send_response_ok(dist2);
        break;

    case '3':
        dist3 = (int)distance[2];
        requestReceived = false;
        Serial.print("distance3:");
        Serial.println(dist3);
        send_response_ok(dist3);
        break;

    case '4':
        dist4 = (int)distance[3];
        requestReceived = false;
        Serial.print("distance4:");
        Serial.println(dist4);
        send_response_ok(dist4);
        break;

    case '5':
        dist5 = (int)distance[4];
        requestReceived = false;
        Serial.print("distance5:");
        Serial.println(dist5);
        send_response_ok(dist5);
        break;

    case '6':
        dist6 = (int)distance[5];
        requestReceived = false;
        Serial.print("distance6:");
        Serial.println(dist6);
        send_response_ok(dist6);
        break;

    default:
        return send_badquery();
    }
}

void SensorServer::send_table_status(char id)
{
    requestReceived = true;
    float s1, s2, s3, s4, s5, s6;
    switch (id)
    {
    case '1':
        if (tableStatus[0] != '\0')
        {
            s1 = (int)tableStatus[0] - 48; // '0' = 48 in decimal
            requestReceived = false;
            Serial.print("status1:");
            Serial.println(s1);
            send_response_ok(s1);
        }
        else
        {
            send_response_ok(nonDisponible);
        }
        break;

    case '2':
        if (tableStatus[1] != '\0')
        {
            s2 = (int)tableStatus[1] - 48;
            requestReceived = false;
            Serial.print("status2:");
            Serial.println(s2);
            send_response_ok(s2);
        }
        else
        {
            send_response_ok(nonDisponible);
        }
        break;

    case '3':
        if (tableStatus[2] != '\0')
        {
            s3 = (int)tableStatus[2] - 48;
            requestReceived = false;
            Serial.print("status3:");
            Serial.println(s3);
            send_response_ok(s3);
        }
        else
        {
            send_response_ok(nonDisponible);
        }
        break;

    case '4':
        if (tableStatus[3] != '\0')
        {
            s4 = (int)tableStatus[3] - 48;
            requestReceived = false;
            Serial.print("status4:");
            Serial.println(s4);
            send_response_ok(s4);
        }
        else
        {
            send_response_ok(nonDisponible);
        }
        break;

    case '5':
        if (tableStatus[4] != '\0')
        {
            s5 = (int)tableStatus[4] - 48;
            requestReceived = false;
            Serial.print("status5:");
            Serial.println(s5);
            send_response_ok(s5);
        }
        else
        {
            send_response_ok(nonDisponible);
        }
        break;

    case '6':
        if (tableStatus[5] != '\0')
        {
            s6 = (int)tableStatus[5] - 48;
            requestReceived = false;
            Serial.print("status6:");
            Serial.println(s6);
            send_response_ok(s6);
        }
        else
        {
            send_response_ok(nonDisponible);
        }
        break;

    default:
        return send_badquery();
    }
}

void SensorServer::on_sensor(String sensor_name)
{
    if (sensor_name.startsWith("distance") && sensor_name.length() == 9)
    {
        on_sensor_distance(sensor_name.charAt(8));
    }
    else if (sensor_name.startsWith("status") && sensor_name.length() == 7)
    {
        send_table_status(sensor_name.charAt(6));
    }
    else if (sensor_name == "temperature")
    {
        mux_com.select_output(0);
        float temperature = dht.readTemperature();
        Serial.print("Temperature:");
        Serial.println(temperature);
        if (!isnan(temperature))
        {
            send_response_ok<float>(temperature);
        }
        else
        {
            send_response_error();
        }
    }
    else if (sensor_name == "humidity")
    {
        mux_com.select_output(0);
        float humidity = dht.readHumidity();
        Serial.print("Humidity:");
        Serial.println(humidity);
        if (!isnan(humidity))
        {
            send_response_ok(humidity);
        }
        else
        {
            send_response_error();
        }
    }
    else if (sensor_name == "motion") // Percentage
    {
        float motion = motion_sensor.get_average_level() * 100.0;
        Serial.print("Motion:");
        Serial.println(motion);
        send_response_ok(motion);
    }
    else if (sensor_name == "sound")
    {
        float sound = sound_sensor.get_average_level() / 1023.0 * 5000.0;
        float volume = 20.0 * (log(sound / 4.9) / log(10));
        Serial.print("Sound:");
        Serial.println(volume);
        send_response_ok(volume);
    }
    else
    {
        send_badquery();
    }
}

void SensorServer::on_config(String config)
{
    if (config.startsWith("readings::motion"))
    {
        send_response_error();
    }
    else if (config.startsWith("readings::sound"))
    {
        send_response_error();
    }
    else
    {
        send_badquery();
    }
}

void SensorServer::on_query(const char *query_str)
{
    Serial.print(F("Incoming query: "));
    Serial.println(query_str);

    String query(query_str);
    //status_led.setColorRGB(0, 20, 40, 82);
    String sensor = "sensors::";
    String config = "config::";

    if (query.startsWith(sensor))
    {
        on_sensor(query.substring(sensor.length()));
    }
    else if (query.startsWith(config))
    {
        on_config(query.substring(config.length()));
    }
    else
    {
        send_badquery();
    }
}

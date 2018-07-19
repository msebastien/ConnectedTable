#include <SensorsDatabase.h>
#include <string>

static const char* sql_init =
"BEGIN IMMEDIATE;"

"CREATE TABLE IF NOT EXISTS sensors.polls ("
	"created INTEGER PRIMARY KEY,"
	"done INTEGER DEFAULT 0,"
	"sent INTEGER DEFAULT 0"
");"

"CREATE TABLE IF NOT EXISTS config.sensors ("
	"identifier TEXT PRIMARY KEY,"
	"common_name TEXT,"
	"sensibility REAL DEFAULT 1,"
	"offset REAL DEFAULT 0,"
	"unit TEXT DEFAULT 'N.A',"
	"chart_color TEXT DEFAULT '#000000'"
");"

"INSERT OR IGNORE INTO config.sensors (identifier, common_name) VALUES "
	"('motion', 'Motion'),"
	"('sound', 'Sound Level'),"
	"('status1', 'Status 1st place'),"
	"('status2', 'Status 2nd place'),"
	"('status3', 'Status 3rd place'),"
	"('status4', 'Status 4th place'),"
	"('status5', 'Status 5th place'),"
	"('status6', 'Status 6th place')"
";"

"INSERT OR IGNORE INTO config.sensors"
"(identifier, common_name, unit) VALUES "
	"('humidity', 'Humidity', '%'),"
	"('temperature', 'Temperature', '°C')"
	"('distance1', 'Raw Distance 1st place', 'mV'),"
	"('distance2', 'Raw Distance 2nd place', 'mV'),"
	"('distance3', 'Raw Distance 3rd place', 'mV'),"
	"('distance4', 'Raw Distance 4th place', 'mV'),"
	"('distance5', 'Raw Distance 5th place', 'mV'),"
	"('distance6', 'Raw Distance 6th place', 'mV'),"
	"('tempIR1', 'Raw Infrared Temperature 1st place', 'mV'),"
	"('tempIR2', 'Raw Infrared Temperature 2nd place', 'mV'),"
	"('tempIR3', 'Raw Infrared Temperature 3rd place', 'mV'),"
	"('tempIR4', 'Raw Infrared Temperature 4th place', 'mV'),"
	"('tempIR5', 'Raw Infrared Temperature 5th place', 'mV'),"
	"('tempIR6', 'Raw Infrared Temperature 6th place', 'mV'),"
";"

"CREATE TABLE IF NOT EXISTS sensors.data ("
	"sensor_identifier TEXT NOT NULL,"
	"data NOT NULL,"
	"poll_time INTEGER NOT NULL,"

	"FOREIGN KEY(poll_time) "
	"REFERENCES polls(created) "
	"ON DELETE CASCADE"
");"

"CREATE TABLE IF NOT EXISTS config.settings ("
	"name TEXT PRIMARY KEY,"
	"value"
");"

// values are in seconds
"INSERT OR IGNORE INTO config.settings (name, value) VALUES "
	"('serial_port', '/dev/ttyS0'),"
	"('node_address', 'NA'),"
	"('lifetime_data', 3600 * 24),"
	"('interval_polling', 14),"
	"('interval_ip', 14),"
	"('interval_lora', 42),"
	"('send_mode', 'ip'),"
	"('server_host', 'localhost'),"
	"('server_port', 5555)"
";"

"DELETE FROM sensors.polls WHERE done=0;"

"COMMIT;";

void SensorsDatabase::init_database(void){
	Statement stmt(db, sql_init);
	do {
		stmt.execute();
	} while(stmt.next_statement());
}

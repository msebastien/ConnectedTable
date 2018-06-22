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
	//"('distance1', 'Distance 1st place', 'cm'),"
	//"('distance2', 'Distance 2nd place', 'cm'),"
	//"('distance3', 'Distance 3rd place', 'cm'),"
	//"('distance4', 'Distance 4th place', 'cm'),"
	//"('distance5', 'Distance 5th place', 'cm'),"
	//"('distance6', 'Distance 6th place', 'cm')"
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
	"('node_address', 'T1'),"
	"('lifetime_data', 3600 * 24),"
	"('interval_polling', 10),"
	"('interval_ip', 10),"
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

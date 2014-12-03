#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <math.h>

#define DB_FILENAME  "/var/local/auriol-db.sl3"
#define M_PI         3.14159265358979323846  /* pi */
#define TEMP_DIFF    10
#define WIND_SAMPLES 10

#ifdef LANGUAGE_ENGLISH
static const char LANG_DB_ERROR_OPENING[] = "ERROR: Can not open database!";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nERROR in Pluviometer query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_ATAH_QUERY[] = "\nERROR in Temperature query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING: Temperature difference out of bonds (%f to %f). Data will NOT be saved!\n";
static const char LANG_DB_CREATE_TBL[] = "ERROR: Could not create pluviometer table! Error Msg: %s.\n%s\n";
static const char LANG_DB_HUMID_DIFF[] = "\nWARNING: Humidity value out of bonds (%i %%). Data will NOT be saved!\n";
#else
static const char LANG_DB_ERROR_OPENING[] = "Can not open database. Dying. Bye bye.";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nSomething went wrong when inserting pluviometer data into DB. Error code: %i.\n";
static const char LANG_DB_ATAH_QUERY[] = "\nSomething went wrong when inserting temperature into DB. Error code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING! Temp jump from %f to %f too big. Temp won't be recorded!\n";
static const char LANG_DB_CREATE_TBL[] = "ERROR: Could not create pluviometer table! Error Msg: %s.\n%s\n";
static const char LANG_DB_HUMID_DIFF[] = "\nWARNING: Humidity value out of bonds (%i %%). Data will NOT be saved!\n";
#endif

static const char * SQL_CREATE_TABLE[] =  {
	"CREATE TABLE IF NOT EXISTS pluviometer( created DATETIME, amount  DECIMAL(10,2));",
	"CREATE TABLE IF NOT EXISTS temperature( created DATETIME, amount  DECIMAL(4,1));",
	"CREATE TABLE IF NOT EXISTS humidity( created DATETIME, amount  TINYINT);",
	"CREATE TABLE IF NOT EXISTS wind( created DATETIME, speed DECIMAL(3,1), gust DECIMAL(3,1), direction SMALLINT );"
};

#define INIT_PREVIOUS(X) previous_value X = { -FLT_MAX, -1 }
typedef struct {
	float value;
	int   time;
} previous_value;

sqlite3 *conn;
int error = 0;

INIT_PREVIOUS( pluviometerPrevious );
INIT_PREVIOUS( temperaturePrevious );
INIT_PREVIOUS( humidityPrevious );
INIT_PREVIOUS( windPrevious );
INIT_PREVIOUS( gustPrevious );

float windAVGSpeed[WIND_SAMPLES];
float windAVGGust[WIND_SAMPLES];
int   windAVGDir[WIND_SAMPLES];
int   windAVGIndex = 1;

void initializeDatabase() {
	char *errMsg = 0;
	int i;
	/* Open database */
    error = sqlite3_open(DB_FILENAME, &conn);
    if (error) {
         puts(LANG_DB_ERROR_OPENING);
         exit(3);
    }
    /* Create database tables, if not exits */
    for ( i=0; i<4; i++ ) {
		error = sqlite3_exec(conn, SQL_CREATE_TABLE[i], 0, 0, &errMsg);
		if (error != SQLITE_OK) {
			printf(LANG_DB_CREATE_TBL, errMsg, SQL_CREATE_TABLE[i]);
			exit(4);
		}
	}
}

void savePluviometer(float amount) {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);
   
	/* Store if value has changed or older than an hour */
    if (pluviometerPrevious.time != local->tm_hour ||
        pluviometerPrevious.value < amount) {

        char query[1024] = " ";
        sprintf(query, "INSERT INTO pluviometer VALUES (datetime('now', 'localtime'), %.2f);", amount);
        error = sqlite3_exec(conn, query, 0, 0, 0);
        if (error != SQLITE_OK) {
            printf(LANG_DB_PLUVIOMETER_QUERY, error);
            exit(5);
        }
    }

    pluviometerPrevious.time  = local->tm_hour;
    pluviometerPrevious.value = amount;
}

void saveTemperature(float temperature) {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);
   
	/* Store if value has changed or older than an hour */
    if (temperaturePrevious.time != local->tm_hour ||
        temperaturePrevious.value != temperature) {
		
		/* Check for invalid values */
        float difference = temperaturePrevious.value - temperature;
        if ((difference < -TEMP_DIFF || difference > TEMP_DIFF) && temperaturePrevious.value != -FLT_MAX) {
            printf(LANG_DB_TEMP_DIFF, temperaturePrevious.value, temperature);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO temperature VALUES (datetime('now', 'localtime'), %.1f);", temperature);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf(LANG_DB_ATAH_QUERY, error);
            exit(6);
        }
    }

    temperaturePrevious.time  = local->tm_hour;
    temperaturePrevious.value = temperature;
}

void saveHumidity(unsigned int humidity) {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);

	/* Store if value has changed or older than an hour */
    if (humidityPrevious.time != local->tm_hour ||
        humidityPrevious.value != humidity) {

		/* Check for invalid values */
        if (humidity <= 0 || humidity > 100) {
            printf(LANG_DB_HUMID_DIFF, humidity);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO humidity VALUES (datetime('now', 'localtime'), %i);", humidity);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf(LANG_DB_PLUVIOMETER_QUERY, error);
            exit(7);
        }
    }

    humidityPrevious.time  = local->tm_hour;
    humidityPrevious.value = humidity;
}

void saveWind(float speed, float gust, unsigned int direction ) {
	unsigned int i;
	unsigned int windDir = 0;
	float x, y, t, windSpeed;
	float windGust = -1.0;
	
	i = windAVGIndex % WIND_SAMPLES;
	windAVGSpeed[i] = speed;
	windAVGGust[i]  = gust;
	windAVGDir[i]   = direction;
	windAVGIndex++;
	
	if ( windAVGIndex < WIND_SAMPLES || i > 0 )
		return;
	
	/* Calculating average wind direction http://www.control.com/thread/1026210133
	 * By M.A.Saghafi on 11 October, 2010 - 8:26 am and M Barnes on 18 May, 2011 - 6:48 am */ 
	for ( i=0; i<WIND_SAMPLES; i++ ) {
		t = M_PI / 180 * windAVGDir[i];
		x += -windAVGSpeed[i] * sin( t );
		y += -windAVGSpeed[i] * cos( t );
		windSpeed += windAVGSpeed[i];
		if ( windAVGGust[i] > windGust )
			windGust = windAVGGust[i];
	}
	
	windSpeed = windSpeed / WIND_SAMPLES;
	x = x / WIND_SAMPLES;
	y = y / WIND_SAMPLES;
	
	if ( x == 0 )
		windDir = 0;
	else if ( x > 0 )
		windDir = 270 - 180 / M_PI * atan( y/x );
	else
		windDir = 90  - 180 / M_PI * atan( y/x );
	
	printf( "Average Wind Speed: %.1f   Wind Gust: %.1f   Wind Direction: %i\n", windSpeed, windGust, windDir );
}
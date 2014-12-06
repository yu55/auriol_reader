/* TODO
 * - Do not calculate average wind direction on sampls where windSpeed = 0.0
*/

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
#define WIND_SAMPLES 125 /* Should be enough for 1 hour*/

#ifdef LANGUAGE_ENGLISH
static const char LANG_DB_ERROR_OPENING[] = "ERROR: Can not open database!";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nERROR in Pluviometer query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_TEMP_QUERY[] = "\nERROR in Temperature query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING: Temperature difference out of bonds (%f to %f). Data will NOT be saved!\n";
static const char LANG_DB_CREATE_TBL[] = "ERROR: Could not create database table! Error Msg: %s.\n%s\n";
static const char LANG_DB_HUMID_DIFF[] = "\nWARNING: Humidity value out of bonds (%i %%). Data will NOT be saved!\n";
static const char LANG_DB_WIND_QUERY[] = "\nERROR in Wind query: SQLite returned Error Code: %i.\n";

#else
static const char LANG_DB_ERROR_OPENING[] = "Can not open database. Dying. Bye bye.";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nSomething went wrong when inserting pluviometer data into DB. Error code: %i.\n";
static const char LANG_DB_TEMP_QUERY[] = "\nSomething went wrong when inserting temperature into DB. Error code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING! Temp jump from %f to %f too big. Temp won't be recorded!\n";
static const char LANG_DB_CREATE_TBL[] = "ERROR: Could not create database table! Error Msg: %s.\n%s\n";
static const char LANG_DB_HUMID_DIFF[] = "\nWARNING: Humidity value out of bonds (%i %%). Data will NOT be saved!\n";
static const char LANG_DB_WIND_QUERY[] = "\nERROR in Wind query: SQLite returned Error Code: %i.\n";
#endif

static const char * SQL_CREATE_TABLE[] =  {
	"CREATE TABLE IF NOT EXISTS pluviometer( created DATETIME, amount  DECIMAL(10,2));",
	"CREATE TABLE IF NOT EXISTS temperature( created DATETIME, amount  DECIMAL(4,1));",
	"CREATE TABLE IF NOT EXISTS humidity( created DATETIME, amount  TINYINT);",
	"CREATE TABLE IF NOT EXISTS wind( created DATETIME, speed DECIMAL(3,1), gust DECIMAL(3,1), direction SMALLINT );"
};

sqlite3 *conn;
int error = 0;
struct tm *local;
time_t t;

void initializeDatabase() {
	char *errMsg = 0;
	int i;
	/* Open database */
    error = sqlite3_open(DB_FILENAME, &conn);
    if (error) {
         fprintf( stderr, LANG_DB_ERROR_OPENING );
         exit(3);
    }
    /* Create database tables, if not exits */
    for ( i=0; i<4; i++ ) {
		error = sqlite3_exec(conn, SQL_CREATE_TABLE[i], 0, 0, &errMsg);
		if (error != SQLITE_OK) {
			fprintf( stderr, LANG_DB_CREATE_TBL, errMsg, SQL_CREATE_TABLE[i] );
			exit(4);
		}
	}
}

void savePluviometer(float amount) {
	static signed int old_hour  = -1;
	static float      old_value = -FLT_MAX;

	t = time(NULL);
	local = localtime(&t);
   
	/* Store if value has changed or older than an hour */
    if ( old_hour != local->tm_hour || old_value < amount ) {
        char query[1024] = " ";
        sprintf(query, "INSERT INTO pluviometer VALUES (datetime('now', 'localtime'), %.2f);", amount);
        error = sqlite3_exec(conn, query, 0, 0, 0);
        if (error != SQLITE_OK) {
            fprintf( stderr, LANG_DB_PLUVIOMETER_QUERY, error);
            exit(5);
        }
    }

    old_hour  = local->tm_hour;
    old_value = amount;
}

void saveTemperature(float temperature) {
	static signed int old_hour = -1;
	static float old_value     = -FLT_MAX;

	t = time(NULL);
	local = localtime(&t);
   
	/* Store if value has changed or older than an hour */
    if ( old_hour != local->tm_hour ||
        old_value != temperature ) {
		
		/* Check for invalid values */
        float difference = old_value - temperature;
        if ((difference < -TEMP_DIFF || difference > TEMP_DIFF) && old_value != -FLT_MAX) {
            printf(LANG_DB_TEMP_DIFF, old_value, temperature);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO temperature VALUES (datetime('now', 'localtime'), %.1f);", temperature);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            fprintf( stderr, LANG_DB_TEMP_QUERY, error);
            exit(6);
        }
    }

    old_hour = local->tm_hour;
    old_value = temperature;
}

void saveHumidity(unsigned int humidity) {
	static signed int old_hour = -1;
	static float old_value     = -FLT_MAX;

   t = time(NULL);
   local = localtime(&t);

	/* Store if value has changed or older than an hour */
    if (old_hour != local->tm_hour ||
        old_value != humidity) {

		/* Check for invalid values */
        if (humidity <= 0 || humidity > 100) {
            fprintf( stderr, LANG_DB_HUMID_DIFF, humidity);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO humidity VALUES (datetime('now', 'localtime'), %i);", humidity);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            fprintf( stderr, LANG_DB_PLUVIOMETER_QUERY, error);
            exit(7);
        }
    }

    old_hour  = local->tm_hour;
    old_value = humidity;
}

void saveWind(float speed, float gust, unsigned int dir ) {
	static time_t     temp_time  = 0;
	static signed int old_hour   = -1;
	static signed int counter    = -1;
	static float      windSpeed[WIND_SAMPLES];
	static float      windGust[WIND_SAMPLES];
	static int        windDir[WIND_SAMPLES];
	
	float x, y, rad;
	unsigned int i;
	
	time( &t );
	local = localtime(&t);
	
	/* Check timestamp on previous reading. Discard old incomplete data */
	if ( difftime( t, temp_time ) > 2.0 ) {
		/* First pass */
		if ( counter < 0 )
			counter = 0;
		else if ( windSpeed[counter] > -1.0 && windGust[counter] > -1.0 )
			counter++;
		i = counter % WIND_SAMPLES;
		windSpeed[i] = -1.0;
		windGust[i]  = -1.0;
		windDir[i]   = -1;
		temp_time    = time( NULL );
	}
	
	/* Store current wind data */
	i = counter % WIND_SAMPLES;
	if ( speed > -1.0 ) {
		windSpeed[i] = speed;
	} else if ( gust > -1.0 ) {
		windGust[i]  = gust;
		windDir[i]   = dir;
	}
	fprintf( stderr, "%i: Speed: %.1f\tGust: %.1f\tDir: %i\n", i, windSpeed[i], windGust[i], windDir[i] );
	
	/* Return if only one value available or less than an hour passed */
	if ( old_hour == local->tm_hour || windSpeed[i] < 0.0 || windGust[i] < 0.0 )
		return;
	
	/* Calculating averages 
	 * Wind dir from http://www.control.com/thread/1026210133
	 * By M.A.Saghafi on 11 October, 2010 - 8:26 am and M Barnes on 18 May, 2011 - 6:48 am */ 
	for ( i = 0; i < ++counter; i++ ) {
		dir = i % WIND_SAMPLES;
		rad = M_PI / 180 * windDir[dir];
		x  += -windSpeed[dir] * sin( rad );
		y  += -windSpeed[dir] * cos( rad );
		speed += windSpeed[dir];
		if ( windGust[dir] > gust )
			gust = windGust[dir];
		windSpeed[dir] = -1.0;
		windGust[dir]  = -1.0;
		windDir[dir]   = -1;
	}
	
	speed = speed / counter;
	x = x / counter;
	y = y / counter;
	
	if ( x == 0 )
		dir = 0;
	else if ( x > 0 )
		dir = 270 - 180 / M_PI * atan( y/x );
	else
		dir = 90  - 180 / M_PI * atan( y/x );
	dir = dir % 360;
	
	char query[80] = " ";
	sprintf(query, "INSERT INTO wind VALUES (datetime('now', 'localtime'), %.1f, %.1f, %i);", speed, gust, dir );
	error = sqlite3_exec(conn, query, 0, 0, 0);

	if (error != SQLITE_OK) {
		fprintf( stderr, LANG_DB_WIND_QUERY, error);
		exit(7);
	}
	
	old_hour = local->tm_hour;
	counter  = -1;
}

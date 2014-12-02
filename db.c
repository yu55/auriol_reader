#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <float.h>

#define DB_FILENAME "/var/local/auriol-db.sl3"
#define TEMP_DIFF 10

#ifdef LANGUAGE_ENGLISH
static const char LANG_DB_ERROR_OPENING[] = "ERROR: Can not open database!";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nERROR in Pluviometer query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_ATAH_QUERY[] = "\nERROR in Temperature query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING: Temperature difference out of bonds (%f to %f). Data will NOT be saved!\n";
static const char LANG_DB_CREATE_TBL_PLUVIOMETER[] = "ERROR: Could not create pluviometer table! Error Code: %s.\n";
static const char LANG_DB_HUMID_DIFF[] = "\nWARNING: Humidity value out of bonds (%i). Data will NOT be saved!\n";
#else
static const char LANG_DB_ERROR_OPENING[] = "Can not open database. Dying. Bye bye.";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nSomething went wrong when inserting pluviometer data into DB. Error code: %i.\n";
static const char LANG_DB_ATAH_QUERY[] = "\nSomething went wrong when inserting temperature into DB. Error code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING! Temp jump from %f to %f too big. Temp won't be recorded!\n";
static const char LANG_DB_CREATE_TBL_PLUVIOMETER[] = "ERROR: Could not create pluviometer table! Error Code: %s.\n";
static const char LANG_DB_HUMID_DIFF[] = "\nWARNING: Humidity value out of bonds (%i). Data will NOT be saved!\n";
#endif

static const char SQL_CREATE_TABLE_PLUVIOMETER[] = "CREATE TABLE IF NOT EXISTS pluviometer ( created DATETIME, amount  DECIMAL(10,2));";
static const char SQL_CREATE_TABLE_TEMPERATURE[] = "CREATE TABLE IF NOT EXISTS temperature ( created DATETIME, amount  DECIMAL(4,1));";
static const char SQL_CREATE_TABLE_HUMIDITY[]    = "CREATE TABLE IF NOT EXISTS humidity ( created DATETIME, amount  TINYINT);";

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

void initializeDatabase() {
	char *errMsg = 0;
	
    error = sqlite3_open(DB_FILENAME, &conn);
    if (error) {
         puts(LANG_DB_ERROR_OPENING);
         exit(3);
    }
    
	error = sqlite3_exec(conn, SQL_CREATE_TABLE_PLUVIOMETER, 0, 0, &errMsg);
	if (error != SQLITE_OK) {
		printf(LANG_DB_CREATE_TBL_PLUVIOMETER, errMsg);
		exit(4);
	}
	
	error = sqlite3_exec(conn, SQL_CREATE_TABLE_TEMPERATURE, 0, 0, &errMsg);
	if (error != SQLITE_OK) {
		printf(LANG_DB_CREATE_TBL_PLUVIOMETER, errMsg);
		exit(5);
	}
	
	error = sqlite3_exec(conn, SQL_CREATE_TABLE_HUMIDITY, 0, 0, &errMsg);
	if (error != SQLITE_OK) {
		printf(LANG_DB_CREATE_TBL_PLUVIOMETER, errMsg);
		exit(6);
	}
	
}

void savePluviometer(float amount) {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);

    if (pluviometerPrevious.time != local->tm_hour ||
        pluviometerPrevious.value < amount) {

        char query[1024] = " ";
        sprintf(query, "INSERT INTO pluviometer VALUES (datetime('now', 'localtime'), %f);", amount);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf(LANG_DB_PLUVIOMETER_QUERY, error);
            exit(1);
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

    if (temperaturePrevious.time != local->tm_hour ||
        temperaturePrevious.value != temperature) {

        float difference = temperaturePrevious.value - temperature;
        if ((difference < -TEMP_DIFF || difference > TEMP_DIFF) && temperaturePrevious.value != -FLT_MAX) {
            printf(LANG_DB_TEMP_DIFF, temperaturePrevious.value, temperature);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO temperature VALUES (datetime('now', 'localtime'), %f);", temperature);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf(LANG_DB_ATAH_QUERY, error);
            exit(1);
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

    if (humidityPrevious.time != local->tm_hour ||
        humidityPrevious.value < humidity) {

        if (humidity <= 0 || humidity > 100) {
            printf(LANG_DB_HUMID_DIFF, humidity);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO humidity VALUES (datetime('now', 'localtime'), %i);", humidity);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf(LANG_DB_PLUVIOMETER_QUERY, error);
            exit(1);
        }
    }

    humidityPrevious.time  = local->tm_hour;
    humidityPrevious.value = humidity;
}

/* http://www.control.com/thread/1026210133
 * By M.A.Saghafi on 11 October, 2010 - 8:26 am
 * and M Barnes on 18 May, 2011 - 6:48 am
 * 
 * Try this with excel:
 * For realvector averaging:
 * Put windspeed and winddirection data in column A and B.
 * Then calculate the components of wind (u and v are in column C and D):
 *   =-A1*SIN(PI()/180*B1)
 *   =-A1*COS(PI()/180*B1)
 * In column E and F (for example,if you have 60 data in one hour) calculate the average of the components of wind:
 *   =AVERAGE(c1:c60)
 *   =AVERAGE(d1:d60)
 * Column G is the average value of the windspeed:
 *   =SQRT(e1*e1+f1*f1))
 * Column H is the average value of the winddirection:
 * =IF(e1=0,(IF(f1=0,"n/a",IF(f1>0;360;0))),IF(e1>0,(270-180/PI()*ATAN(f1/e1)),(90-180/PI()*ATAN(f1/e1))))
**/
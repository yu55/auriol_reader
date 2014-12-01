#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <float.h>

#define DB_FILENAME "database.sl3"

#ifdef LANGUAGE_ENGLISH
static const char LANG_DB_ERROR_OPENING[] = "ERROR: Can not open database!";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nERROR in Pluviometer query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_ATAH_QUERY[] = "\nERROR in Temperature query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING: Temperature difference out of bonds (%f to %f). Data will NOT be saved!\n"
#else
static const char LANG_DB_ERROR_OPENING[] = "Can not open database. Dying. Bye bye.";
static const char LANG_DB_PLUVIOMETER_QUERY[] = "\nSomething went wrong when inserting pluviometer data into DB. Error code: %i.\n";
static const char LANG_DB_ATAH_QUERY[] = "\nSomething went wrong when inserting temperature into DB. Error code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] = "\nWARNING! Temp jump from %f to %f too big. Temp won't be recorded!\n"
#endif

sqlite3 *conn;
int error = 0;

float pluviometerPreviousAmountInMilimeters = -1;
int pluviometerPreviousTmHour = -1;

float atahPreviousTemperatureInC = -FLT_MAX;
int atahPreviousTmHour = -1;

void initializeDatabase() {
    error = sqlite3_open(DB_FILENAME, &conn);
    if (error) {
         puts(LANG_DB_ERROR_OPENING);
         exit(0);
    }
}

void savePluviometer(float amountInMilimeters) {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);

    if (pluviometerPreviousTmHour != local->tm_hour ||
        pluviometerPreviousAmountInMilimeters < amountInMilimeters) {

        char query[1024] = " ";
        sprintf(query, "INSERT INTO pluviometer VALUES (datetime('now', 'localtime'), %f);", amountInMilimeters);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf(LANG_DB_PLUVIOMETER_QUERY, error);
            exit(1);
        }
    }

    pluviometerPreviousTmHour = local->tm_hour;
    pluviometerPreviousAmountInMilimeters = amountInMilimeters;
}

void saveAnemometerTemperatureAndHumidity(float temperatureInC) {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);

    if (atahPreviousTmHour != local->tm_hour ||
        atahPreviousTemperatureInC != temperatureInC) {

        float difference = atahPreviousTemperatureInC - temperatureInC;
        if ((difference < -1 || difference > 1) && atahPreviousTemperatureInC != -FLT_MAX) {
            printf(LANG_DB_TEMP_DIFF, atahPreviousTemperatureInC, temperatureInC);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO anemometer VALUES (datetime('now', 'localtime'), %f);", temperatureInC);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf(LANG_DB_ATAH_QUERY, error);
            exit(1);
        }
    }

    atahPreviousTmHour = local->tm_hour;
    atahPreviousTemperatureInC = temperatureInC;
}

#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <float.h>

#define DB_FILENAME "database.sl3"

sqlite3 *conn;
int error = 0;

float pluviometerPreviousAmountInMilimeters = -1;
int pluviometerPreviousTmHour = -1;

float atahPreviousTemperatureInC = -FLT_MAX;
int atahPreviousTmHour = -1;

void initializeDatabase() {
    error = sqlite3_open(DB_FILENAME, &conn);
    if (error) {
         puts("Can not open database. Dying. Bye bye.");
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
            printf("\nSomething went wrong when inserting pluviometer data into DB. Error code: %i.\n", error);
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
        if ((difference < -10 || difference > 10) && atahPreviousTemperatureInC != -FLT_MAX) {
            printf("\nWARNING! Temp jump from %f to %f too big. Temp won't be recorded!\n", atahPreviousTemperatureInC, temperatureInC);
            return;
        }

        char query[1024] = " ";
        sprintf(query, "INSERT INTO anemometer VALUES (datetime('now', 'localtime'), %f);", temperatureInC);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            printf("\nSomething went wrong when inserting temperature into DB. Error code: %i.\n", error);
            exit(1);
        }
    }

    atahPreviousTmHour = local->tm_hour;
    atahPreviousTemperatureInC = temperatureInC;
}

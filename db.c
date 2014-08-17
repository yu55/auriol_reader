#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

#define DB_FILENAME "database.sl3"

sqlite3 *conn;
int error = 0;

float pluviometerPreviousAmountInMilimeters = 0;
int pluviometerPreviousTmMin = -1;
int pluviometerPreviousTmHour = -1;

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

    if (pluviometerPreviousTmMin != local->tm_min ||
        pluviometerPreviousTmHour != local->tm_hour ||
        pluviometerPreviousAmountInMilimeters != amountInMilimeters) {

        char query[1024] = " ";
        sprintf(query, "insert into pluviometer values (datetime('now', 'localtime'), %f);", amountInMilimeters);
        error = sqlite3_exec(conn, query, 0, 0, 0);

        if (error != SQLITE_OK) {
            puts("Something went wrong when inserting pluviometer data. Dying. Bye bye.");
            exit(0);
        }
    }

    pluviometerPreviousTmMin = local->tm_min;
    pluviometerPreviousTmHour = local->tm_hour;
    pluviometerPreviousAmountInMilimeters = amountInMilimeters;
}

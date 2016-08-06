#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <math.h>

#define DB_FILENAME  "/var/local/auriol-db.sl3"
#define M_PI         3.14159265358979323846	/* pi */
#define TEMP_DIFF    10
#define WIND_SAVE_INTERVALL 3600.0	/* (double) Intervall to save wind data in seconds */
#define WIND_SAMPLES 120	/* WIND_SAMPLES = WIND_SAVE_INTERVALL / 30 */

#ifdef LANGUAGE_ENGLISH
static const char LANG_DB_ERROR_OPENING[] =
    "ERROR: Can not open database!";
static const char LANG_DB_PLUVIOMETER_QUERY[] =
    "\nERROR in Pluviometer query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_TEMP_QUERY[] =
    "\nERROR in Temperature query: SQLite returned Error Code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] =
    "\nWARNING: Temperature difference out of bonds (%f to %f). Data will NOT be saved!\n";
static const char LANG_DB_CREATE_TBL[] =
    "ERROR: Could not create database table! Error Msg: %s.\n%s\n";
static const char LANG_DB_HUMID_DIFF[] =
    "\nWARNING: Humidity value out of bonds (%i %%). Data will NOT be saved!\n";
static const char LANG_DB_WIND_QUERY[] =
    "\nERROR in Wind query: SQLite returned Error Code: %i.\n\"%s\"\n";

#else
static const char LANG_DB_ERROR_OPENING[] =
    "Can not open database. Dying. Bye bye.";
static const char LANG_DB_PLUVIOMETER_QUERY[] =
    "\nSomething went wrong when inserting pluviometer data into DB. Error code: %i.\n";
static const char LANG_DB_TEMP_QUERY[] =
    "\nSomething went wrong when inserting temperature into DB. Error code: %i.\n";
static const char LANG_DB_TEMP_DIFF[] =
    "\nWARNING! Temp jump from %f to %f too big. Temp won't be recorded!\n";
static const char LANG_DB_CREATE_TBL[] =
    "ERROR: Could not create database table! Error Msg: %s.\n%s\n";
static const char LANG_DB_HUMID_DIFF[] =
    "\nWARNING: Humidity value out of bonds (%i %%). Data will NOT be saved!\n";
static const char LANG_DB_WIND_QUERY[] =
    "\nERROR in Wind query: SQLite returned Error Code: %i.\n\"%s\"\n";
#endif

static const char *SQL_CREATE_TABLE[] = {
    "CREATE TABLE IF NOT EXISTS pluviometer( created DATETIME, amount  DECIMAL(10,2));",
    "CREATE TABLE IF NOT EXISTS temperature( created DATETIME, amount  DECIMAL(4,1));",
    "CREATE TABLE IF NOT EXISTS humidity( created DATETIME, amount  TINYINT);",
    "CREATE TABLE IF NOT EXISTS wind( created DATETIME, speed DECIMAL(3,1), gust DECIMAL(3,1), direction SMALLINT );"
};

static const char *SQL_CREATE_INDEX[] = {
    "CREATE INDEX IF NOT EXISTS ix_pluviometer_created ON pluviometer(created);",
    "CREATE INDEX IF NOT EXISTS ix_temperature_created ON temperature(created);",
    "CREATE INDEX IF NOT EXISTS ix_humidity_created ON humidity(created);",
    "CREATE INDEX IF NOT EXISTS ix_wind_created ON wind(created);"
};


sqlite3 *conn;
int error = 0;
struct tm *local;
time_t t;

void initializeDatabase()
{
    char *errMsg = 0;
    int i;
    /* Open database */
    error = sqlite3_open(DB_FILENAME, &conn);
    if (error) {
	fprintf(stderr, LANG_DB_ERROR_OPENING);
	exit(3);
    }
    /* Create database tables and indices, if not exits */
    for (i = 0; i < 4; i++) {
	error = sqlite3_exec(conn, SQL_CREATE_TABLE[i], 0, 0, &errMsg);
	if (error != SQLITE_OK) {
	    fprintf(stderr, LANG_DB_CREATE_TBL, errMsg,
		    SQL_CREATE_TABLE[i]);
	    exit(4);
	}

	error = sqlite3_exec(conn, SQL_CREATE_INDEX[i], 0, 0, &errMsg);
	if (error != SQLITE_OK) {
	    fprintf(stderr, LANG_DB_CREATE_TBL, errMsg,
		    SQL_CREATE_INDEX[i]);
	    exit(4);
	}
    }
}

void savePluviometer(float amount)
{
    static signed int old_hour = -1;
    static float old_value = -FLT_MAX;

    t = time(NULL);
    local = localtime(&t);

    /* Store if value has changed or older than an hour */
    if (old_hour != local->tm_hour || old_value < amount) {
	char query[1024] = " ";
	sprintf(query,
		"INSERT INTO pluviometer VALUES (datetime('now', 'localtime'), %.2f);",
		amount);
	error = sqlite3_exec(conn, query, 0, 0, 0);
	if (error != SQLITE_OK) {
	    fprintf(stderr, LANG_DB_PLUVIOMETER_QUERY, error);
	    exit(5);
	}
    }

    old_hour = local->tm_hour;
    old_value = amount;
}

void saveTemperature(float temperature)
{
    static signed int t_old_min = -1;
    static float old_value = -FLT_MAX;

    t = time(NULL);
    local = localtime(&t);

    /* Store if value has changed or not from same minute */
    if (t_old_min != local->tm_min || old_value != temperature) {

	/* Check for invalid values */
	float difference = old_value - temperature;
	if ((difference < -TEMP_DIFF || difference > TEMP_DIFF)
	    && old_value != -FLT_MAX) {
	    printf(LANG_DB_TEMP_DIFF, old_value, temperature);
	    return;
	}

	char query[1024] = " ";
	sprintf(query,
		"INSERT INTO temperature VALUES (datetime('now', 'localtime'), %.1f);",
		temperature);
	error = sqlite3_exec(conn, query, 0, 0, 0);

	if (error != SQLITE_OK) {
	    fprintf(stderr, LANG_DB_TEMP_QUERY, error);
	    exit(6);
	}
    }

    t_old_min = local->tm_min;
    old_value = temperature;
}

void saveHumidity(unsigned int humidity)
{
    static signed int h_old_min = -1;
    static float old_value = -FLT_MAX;

    t = time(NULL);
    local = localtime(&t);

    /* Store if value has changed or not from same minute */
    if (h_old_min != local->tm_min || old_value != humidity) {

	/* Check for invalid values */
	if (humidity <= 0 || humidity > 100) {
	    fprintf(stderr, LANG_DB_HUMID_DIFF, humidity);
	    return;
	}

	char query[1024] = " ";
	sprintf(query,
		"INSERT INTO humidity VALUES (datetime('now', 'localtime'), %i);",
		humidity);
	error = sqlite3_exec(conn, query, 0, 0, 0);

	if (error != SQLITE_OK) {
	    fprintf(stderr, LANG_DB_PLUVIOMETER_QUERY, error);
	    exit(7);
	}
    }

    h_old_min = local->tm_min;
    old_value = humidity;
}

void saveWind(float speed, float gust, unsigned int dir)
{
    static time_t temp_time = 0;
    static time_t old_time = 0;
    static signed int counter = 0;
    static float windSpeed[WIND_SAMPLES] = { -1.0 };
    static float windGust[WIND_SAMPLES] = { -1.0 };
    static int windDir[WIND_SAMPLES] = { -1 };
    static int rowid = 0;
    static int saved = 0;

    float x, y, rad;
    unsigned int i;

    time(&t);

    /* Check a new reading (aprox. every 30s). Discard incomplete data */
    if (difftime(t, temp_time) > 20.0) {
	if (windSpeed[counter] > -1.0 && windGust[counter] > -1.0)
	    counter++;
	i = counter % WIND_SAMPLES;
	windSpeed[i] = -1.0;
	windGust[i] = -1.0;
	windDir[i] = -1;
	temp_time = time(NULL);
	saved = 0;
    }

    /* Store current wind data */
    i = counter % WIND_SAMPLES;
    if (speed > -1.0) {
	windSpeed[i] = speed;
    } else if (gust > -1.0) {
	windGust[i] = gust;
	windDir[i] = dir;
    }

    /* Return if only one value available or already saved */
    if (windSpeed[i] < 0.0 || windGust[i] < 0.0 || saved == 1)
	return;

    /* Calculate averages 
     * Wind dir from http://www.control.com/thread/1026210133
     * By M.A.Saghafi on 11 October, 2010 - 8:26 am and M Barnes on 18 May, 2011 - 6:48 am */
    ++counter;
    speed = 0.0;
    for (i = 0; i < counter; i++) {
#if DEBUG > 2
	fprintf(stderr,
		"[AVG] %i: Speed: %.1f\tGust: %.1f\tDir: %i\tc: %i\n", i,
		windSpeed[i], windGust[i], windDir[i], counter);
#endif
	dir = i % WIND_SAMPLES;
	rad = M_PI / 180 * windDir[dir];
	x += -windSpeed[dir] * sin(rad);
	y += -windSpeed[dir] * cos(rad);
	speed += windSpeed[dir];
	if (windGust[dir] > gust)
	    gust = windGust[dir];
    }

    speed = speed / counter;
    x = x / counter;
    y = y / counter;

    if (x == 0)
	dir = 0;
    else if (x > 0)
	dir = 270 - 180 / M_PI * atan(y / x);
    else
	dir = 90 - 180 / M_PI * atan(y / x);
    dir = dir % 360;

    /* Update row if time < WIND_SAVE_INTERVALL */
    char query[128] = " ";
    if (difftime(t, old_time) < WIND_SAVE_INTERVALL && rowid > 0) {
	sprintf(query,
		"UPDATE wind SET created=datetime('now','localtime'), speed=%.1f, gust=%.1f, direction=%i WHERE rowid=%i;",
		speed, gust, dir, rowid);
	error = sqlite3_exec(conn, query, 0, 0, 0);
	if (error != SQLITE_OK)
	    fprintf(stderr, LANG_DB_WIND_QUERY, error, query);

	--counter;
	/* Insert a new row */
    } else {
	sprintf(query,
		"INSERT INTO wind VALUES (datetime('now', 'localtime'), %.1f, %.1f, %i);",
		speed, gust, dir);
	error = sqlite3_exec(conn, query, 0, 0, 0);
	if (error != SQLITE_OK)
	    fprintf(stderr, LANG_DB_WIND_QUERY, error, query);

	rowid = (int) sqlite3_last_insert_rowid(conn);
	counter = 0;
	old_time = time(NULL);
	windGust[0] = -1.0;
	windDir[0] = -1.0;
    }
    saved = 1;
#if DEBUG > 1
    fprintf(stderr, "Query: %s\n", query);
#endif
}

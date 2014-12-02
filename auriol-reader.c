#define LANGUAGE_ENGLISH

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <wiringPi.h>
#include "db.h"

/* #define ARRAY_SIZE 4800000 */
#define SYNCHRO_LENGTH 178
#define SEPARATOR_LENGTH 9
#define ZERO_LENGTH 38
#define ONE_LENGTH 78
#define LENGTHS_MARGIN 5

#ifdef LANGUAGE_ENGLISH
static const char LANG_PROGRAM_TITLE[] = "433 MHz Wireless Weather Station Decoder running on Raspberry Pi.\n";
static const char LANG_BATTERY_OK[] = "        Battery: OK\n";
static const char LANG_BATTERY_REPLACE[] = "        Battery: Need replacing (<2.6V)\n";
static const char LANG_TRANS_FILE_OPEN_ERR[] = "Error: Could not open transmission data file!";
static const char LANG_TRANS_FILE_END[] = "End of transmission data.";
static const char LANG_INFO_PLUVIOMETER[] = "Rain: %.2f mm";
static const char LANG_INFO_WIND_AVG[] = "Wind Speed: %.1f m/s";
static const char LANG_INFO_WIND_DIR_GUST[] = "Wind Direction: %i deg        Wind Gust: %.1f m/s";
static const char LANG_INFO_TEMP_HUMIDITY[] = "Temperature: %.1f C        Humidity: %i %%";
static const char LANG_INFO_CRC_DIFFER[] = "ReceivedChecksum=%02x CalculatedChecksum=%02x Equal=%d\n";
static const char LANG_WARNING_CRC[] = "WARNING! Checksum failed. Data will NOT be saved\n";
static const char LANG_DATE_TIME[] = "[%i-%02i-%02i %02i:%02i:%02i] ";
#else
static const char LANG_PROGRAM_TITLE[] = "Dekoder czujnikow bezprzewodowych 433 MHz na Raspberry Pi uruchomiony.\n";
static const char LANG_BATTERY_OK[] = "        Bateria: OK\n";
static const char LANG_BATTERY_REPLACE[] = "        Bateria: do wymiany (napiecie < 2.6V)\n";
static const char LANG_TRANS_FILE_OPEN_ERR[] = "Could not open file with transmission data. Terminating. Good-bye!";
static const char LANG_TRANS_FILE_END[] = "Reached end of file with transmission data. Terminating. Good-bye!";
static const char LANG_INFO_PLUVIOMETER[] = "Deszczomierz: %.2f mm";
static const char LANG_INFO_WIND_AVG[] = "Srednia predkosc wiatru: %.2f m/s";
static const char LANG_INFO_WIND_DIR_GUST[] = "Kierunek wiatru: %i stopni        Poryw: %.2f m/s";
static const char LANG_INFO_TEMP_HUMIDITY[] = "Temperatura: %.2f C        Wilgotnosc: %i %%";
static const char LANG_INFO_CRC_DIFFER[] = "readedChecksum=%02x computedChecksum=%02x equal=%d\n";
static const char LANG_WARNING_CRC[] = "WARNING! Checksum not confirmed. Data will NOT be saved in database\n";
static const char LANG_DATE_TIME[] = "[%i-%02i-%02i %02i:%02i:%02i] ";
#endif

void openFileWithTransmissionData();
unsigned char readLevel();
int findEncodedBitLength(unsigned char level);
void resetRecording();
void printArray();
void decodeBitLength(int length);
void decodeArray();
void decodePluviometer();
void decodeWindData();
bool combinedSensorChecksumConfirmed();
void printTime();

char *filename;
FILE *pFile = NULL;
int globalLevelsCounter = 0;
int levelsCounter = 0;
int levelOneCounter = 0;
unsigned char previousEncodedBitInRange = 0;
unsigned char recording = 0;
unsigned char encodedBits[36] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char encodedBitsIndex = 0;

int main(int argc, char *argv[])
{
    filename = argv[1];

    initializeDatabase();

/*    unsigned char levels[ARRAY_SIZE]; */

    wiringPiSetup();
    /* openFileWithTransmissionData(); */

    printTime();
    printf(LANG_PROGRAM_TITLE);

/*    while(globalLevelsCounter < ARRAY_SIZE) */
    while(1)
    {
        int level = digitalRead(2);
/*        unsigned char level = readLevel(); */

        int bitLength = findEncodedBitLength(level);
/*
        if (bitLength != -1) {
            printf("%i   %i\n", bitLength, count);
        }
*/
        decodeBitLength(bitLength);

/*        levels[globalLevelsCounter] = level; */

        delayMicroseconds(50);

        globalLevelsCounter++;
    }
/*
    count = 0;

    while(globalLevelsCounter < ARRAY_SIZE)
    {
        printf("%i", levels[globalLevelsCounter]);

        count++;
    }
*/
    return 0;
}

/* Open file with transmission data */
void openFileWithTransmissionData() {
    pFile = fopen(filename, "rt");
    if (pFile == NULL) {
        puts(LANG_TRANS_FILE_OPEN_ERR);
        exit(1);
    }
}

/* Read GPIO level */
unsigned char readLevel() {
    int c = fgetc(pFile);
    if (c != EOF) {
        return c - '0';
    } else {
        puts(LANG_TRANS_FILE_END);
        exit(2);
    }
}

/* Analyze transmission bit after bit */
int findEncodedBitLength(unsigned char level) {
       levelsCounter++;
       if (levelsCounter > SYNCHRO_LENGTH + LENGTHS_MARGIN + SEPARATOR_LENGTH + LENGTHS_MARGIN) {
           levelsCounter = 1;
           resetRecording();
           previousEncodedBitInRange = 1;
       }

       int encodedBitLength = -1;
       if (level == 0) {
           if (levelOneCounter > SEPARATOR_LENGTH - LENGTHS_MARGIN && levelOneCounter < SEPARATOR_LENGTH + LENGTHS_MARGIN) {
               if (previousEncodedBitInRange) {
                   encodedBitLength = levelsCounter - levelOneCounter;
               }
               levelsCounter = 0;
               previousEncodedBitInRange = 1;
           }
           levelOneCounter = 0;
       } else {
           levelOneCounter++;
       }
       return encodedBitLength;
}

void resetRecording() {
   /*printArray();*/

   recording = 0;
   memset(encodedBits, 2, sizeof(encodedBits[0]) * 36);
   encodedBitsIndex = 0;
}

void decodeBitLength(int length) {
/*       if (length > -1) {
          printf("%i %i\n", length, globalLevelsCounter);
       }*/
       if (length < ZERO_LENGTH - LENGTHS_MARGIN) {
           return;
       }
       if (length > SYNCHRO_LENGTH - LENGTHS_MARGIN*2 && length < SYNCHRO_LENGTH + LENGTHS_MARGIN*2 && !recording) {
           resetRecording();
           recording = 1;
       } else if (length > ONE_LENGTH - LENGTHS_MARGIN && length < ONE_LENGTH + LENGTHS_MARGIN && recording) {
           encodedBits[encodedBitsIndex++] = 1;
       } else if (length > ZERO_LENGTH - LENGTHS_MARGIN && length < ZERO_LENGTH + LENGTHS_MARGIN && recording) {
           encodedBits[encodedBitsIndex++] = 0;
       } else if (length > SYNCHRO_LENGTH - LENGTHS_MARGIN*2 && length < SYNCHRO_LENGTH + LENGTHS_MARGIN*2 && recording) {
           decodeArray();
           resetRecording();
           recording = 1;
       } else if (recording) {
           resetRecording();
       }
}

void printArray() {
   if (encodedBitsIndex == 0) {
        return;
   }

   printTime();

   int i = 0;
   for (i=0; i<36; i++) {
       printf("%i", encodedBits[i]);
   }
   printf("\n");
   /*
   printf(" globalLevelsCounter=%i\n", globalLevelsCounter);
   */
}

void decodeArray() {
   if (encodedBitsIndex < 36) {
       return;
   }
   printArray();
   decodePluviometer();
   decodeWindData();
}

void decodePluviometer() {
    if (encodedBits[9] && encodedBits[10] && !encodedBits[11] && encodedBits[12] && encodedBits[13] && !encodedBits[14] && !encodedBits[15]) {
        unsigned int rain = 0;
        int i;
        for (i=16; i<32; i++) {
           rain |= encodedBits[i] << (i - 16);
        }

        printTime();
        float rainFinal = (float)rain/4;
        printf(LANG_INFO_PLUVIOMETER, (float)rainFinal);
        savePluviometer(rainFinal);

        if (encodedBits[8]) {
           printf(LANG_BATTERY_REPLACE);
        } else {
           printf(LANG_BATTERY_OK);
        }
    }
}

void decodeWindData() {
    /* Average Wind Speed */
	if (encodedBits[9] && encodedBits[10] && encodedBits[12] && !encodedBits[13] && !encodedBits[14] && !encodedBits[15] && !encodedBits[16]
           && !encodedBits[17] && !encodedBits[18] && !encodedBits[19] && !encodedBits[20] && !encodedBits[21] && !encodedBits[22] && !encodedBits[23]) {
        unsigned int windAverageSpeed = 0;
        int i;
        for (i=24; i<32; i++) {
           windAverageSpeed |= encodedBits[i] << (i - 24);
        }

        printTime();
        printf(LANG_INFO_WIND_AVG, (float)windAverageSpeed/5);

        if (encodedBits[8]) {
           printf(LANG_BATTERY_REPLACE);
        } else {
           printf(LANG_BATTERY_OK);
        }
    
    /* Wind gust & direction */
    } else if (encodedBits[9] && encodedBits[10] && encodedBits[12] && encodedBits[13] && encodedBits[14]) {
        unsigned int direction = 0;
        unsigned int windGust = 0;
        int i;
        for (i=15; i<23; i++) {
           direction |= encodedBits[i] << (i - 15);
        }
        for (i=24; i<32; i++) {
           windGust |= encodedBits[i] << (i - 24);
        }

        printTime();
        printf(LANG_INFO_WIND_DIR_GUST, direction, (float)windGust/5);

        if (encodedBits[8]) {
           printf(LANG_BATTERY_REPLACE);
        } else {
           printf(LANG_BATTERY_OK);
        }
    
    /* Temperature & Humidity */
    } else if (!encodedBits[9] || !encodedBits[10]) {
        int temperature = 0;
        int i;
        for (i=12; i<23; i++) {
           temperature |= encodedBits[i] << (i - 12);
        }
        if (encodedBits[23]) {
           temperature = -2048 + temperature;
        }
        float temperatureFinal = (float)temperature/10;

        unsigned int humidityOnes = 0;
        for (i=24; i<28; i++) {
           humidityOnes |= encodedBits[i] << (i - 24);
        }

        unsigned int humidityTens = 0;
        for (i=28; i<32; i++) {
           humidityTens |= encodedBits[i] << (i - 28);
        }

        unsigned int humidity = humidityTens * 10 + humidityOnes;

        printTime();
        printf(LANG_INFO_TEMP_HUMIDITY, temperatureFinal, humidity);

        if (encodedBits[8]) {
           printf(LANG_BATTERY_REPLACE);
        } else {
           printf(LANG_BATTERY_OK);
        }

        if (combinedSensorChecksumConfirmed()) {
            saveTemperature(temperatureFinal);
        } else {
            printf(LANG_WARNING_CRC);
        }
    }
}

bool combinedSensorChecksumConfirmed() {
    unsigned char computedChecksum = 0x0F;
    int i = 0, j = 0;
    for (i=0; i<32; i+=4) {
        unsigned char nibble = 0;
        for (j=0; j<4; j++) {
            nibble |= encodedBits[i+j] << (j);
        }
/*
        printf("%02x (%d) - %02x (%d) = ", computedChecksum, computedChecksum, nibble, nibble);
*/
        computedChecksum -= nibble;
/*
        printf("%02x (%d)\n", computedChecksum, computedChecksum);
*/
    }
    computedChecksum &= 0x0F;

    unsigned int readedChecksum = 0x00;
    for (i=32; i<36; i++) {
        readedChecksum |= encodedBits[i] << (i - 32);
    }

    bool checksumsAreEqual = (readedChecksum == computedChecksum);

    if (!checksumsAreEqual) {
        printTime();
        printf(LANG_INFO_CRC_DIFFER, readedChecksum, computedChecksum, checksumsAreEqual);
    }

    return checksumsAreEqual;
}

void printTime() {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);
   printf(LANG_DATE_TIME, (local->tm_year + 1900), (local->tm_mon) + 1, local->tm_mday, local->tm_hour,
       local->tm_min, local->tm_sec);
}



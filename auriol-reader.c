#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>

/* #define ARRAY_SIZE 4800000 */
#define SYNCHRO_LENGTH 178
#define SEPARATOR_LENGTH 9
#define ZERO_LENGTH 38
#define ONE_LENGTH 78
#define LENGTHS_MARGIN 4

void openFileWithTransmissionData();
unsigned char readLevel();
int findEncodedBitLength(unsigned char level);
void resetRecording();
void printArray();
void decodeBitLength(int length);
void decodeArray();
void decodePluviometer();
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

/*    unsigned char levels[ARRAY_SIZE]; */

    wiringPiSetup();
    /* openFileWithTransmissionData(); */

    printTime();
    printf("Dekoder czujnikow bezprzewodowych 433 MHz na Raspberry Pi uruchomiony.\n");

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
        puts("Could not open file with transmission data. Terminating. Good-bye!");
        exit(1);
    }
}

/* Read GPIO level */
unsigned char readLevel() {
    int c = fgetc(pFile);
    if (c != EOF) {
        return c - '0';
    } else {
        puts("Reached end of file with transmission data. Terminating. Good-bye!");
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
/*   printArray(); */

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
       if (length > SYNCHRO_LENGTH - LENGTHS_MARGIN && length < SYNCHRO_LENGTH + LENGTHS_MARGIN && !recording) {
           resetRecording();
           recording = 1;
       } else if (length > ONE_LENGTH - LENGTHS_MARGIN && length < ONE_LENGTH + LENGTHS_MARGIN && recording) {
           encodedBits[encodedBitsIndex++] = 1;
       } else if (length > ZERO_LENGTH - LENGTHS_MARGIN && length < ZERO_LENGTH + LENGTHS_MARGIN && recording) {
           encodedBits[encodedBitsIndex++] = 0;
       } else if (length > SYNCHRO_LENGTH - LENGTHS_MARGIN && length < SYNCHRO_LENGTH + LENGTHS_MARGIN && recording) {
           decodeArray();
           resetRecording();
           recording = 1;
       } else if (recording) {
           resetRecording();
       }
}

void printArray() {
   int i = 0;
   for (i=0; i<36; i++) {
       printf("%i", encodedBits[i]);
   }
   printf(" globalLevelsCounter=%i\n", globalLevelsCounter);
}

void decodeArray() {
   if (encodedBitsIndex < 36) {
       return;
   }
   decodePluviometer();
}

void decodePluviometer() {
    if (encodedBits[9] && encodedBits[10] && !encodedBits[11] && encodedBits[12] && encodedBits[13] && !encodedBits[14] && !encodedBits[15]) {
        unsigned int rain = 0;
        int i;
        for (i=16; i<32; i++) {
           rain |= encodedBits[i] << (i - 16);
        }

        printTime();
        printf("        Deszczomierz: %.2f mm", (float)rain/4);

        if (encodedBits[8]) {
           printf("        Bateria: do wymiany (napiecie < 2.6V)\n");
        } else {
           printf("        Bateria: OK\n");
        }
    }
}

void printTime() {
   struct tm *local;
   time_t t;

   t = time(NULL);
   local = localtime(&t);
   printf("[%i-%02i-%02i %02i:%02i:%02i] ", (local->tm_year + 1900), local->tm_mon, local->tm_mday, local->tm_hour,
       local->tm_min, local->tm_sec);
}



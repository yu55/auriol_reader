#include <stdio.h>
#include <wiringPi.h>

#define ARRAY_SIZE 4800000

int main()
{
    unsigned char levels[ARRAY_SIZE];

    wiringPiSetup();

    int count = 0;

    while( count < ARRAY_SIZE )
    {
        int level = digitalRead( 2 );

        levels[count] = level;

        delayMicroseconds( 50 );

        count++;
    }

    count = 0;

    while( count < ARRAY_SIZE )
    {
        printf( "%i", levels[count] );

        count++;
    }

    return 0;
}

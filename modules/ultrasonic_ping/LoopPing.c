#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <wiringPi.h>
#define TRIGGER_PIN 23
#define ECHO_PIN  24
#define TIMEOUT 999 /* any value other than LOW or HIGH */

// get us
unsigned long GetTimeStamp() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*(unsigned long )1000000+tv.tv_usec;
}

int main (int argc, char *argv[])
{
    unsigned long  pulsewidth;
    if (wiringPiSetupGpio () == -1)
    {
        fprintf (stderr, “Can’t initialise wiringPi: %s\n”, strerror (errno)) ;
        return 1 ;
    }
    
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    int i;
    for (i = 0; i < 100; i++)
    {
        //# Set trigger to False (Low)
        digitalWrite(TRIGGER_PIN, LOW);
        //# Allow module to settle
        delay(500);
        
        //# Send 10us pulse to trigger
        /* trigger reading */
        digitalWrite(TRIGGER_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGGER_PIN, LOW);
        
        /* wait for reading to start */
        //waitforpin(ECHO_PIN, LOW, 5000); /* 5 ms timeout */
        
        unsigned long now, start;
        start = GetTimeStamp();
        while (digitalRead(ECHO_PIN) == LOW) {
            start = GetTimeStamp();
        }
        
        while (digitalRead(ECHO_PIN) == HIGH) {
            now = GetTimeStamp();
        }
        
        //printf(“start at %lu \n”, start);
        //printf(“now at %lu \n”, now);
        
        pulsewidth = now - start;
        
        if (pulsewidth < 60000L)
        {
            /* valid reading code */
            //printf(“echo at %d micros\n”, pulsewidth);
            
            float timeInSecond = (float)(pulsewidth / 1000000.00F);
            //printf(“elaped = %d s\n”, timeInSecond);
            float distance = timeInSecond * 34000.00F;
            distance = distance / 2.00;
            printf(“%f\n”, distance);		
        }	 
        else	
        {	 
            /* no object detected code */  
            //printf(“echo timed out\n”);
        }	 
    }  
}
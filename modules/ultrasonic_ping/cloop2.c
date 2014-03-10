#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <wiringPi.h>
#define TRIGGER_PIN 18
#define ECHO_PIN  23
#define TIMEOUT 999 /* any value other than LOW or HIGH */


unsigned long GetTimeStamp() {
    volatile struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(long)1000000+tv.tv_usec;
}

// timeout in us
long waitforpin(int pin, int level, unsigned long timeout)
{
    volatile register unsigned long now, start;
    volatile unsigned long micros = 0;
    start = GetTimeStamp();
    while (1)
    {
        now = GetTimeStamp();
        micros = now - start;
        if (digitalRead(pin) == level) return micros;
        if (micros > timeout) return micros;
    }
    return micros;
}

int main (int argc, char *argv[])
{
    unsigned long pulsewidth;
    
    if (wiringPiSetupGpio () == -1)
    {
        fprintf (stderr, "Can't initialise wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }
    
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    int i;
    for (i = 0; i < 10; i++)
    {
        pulsewidth = 0;
        
        //# Set trigger to False (Low)
        digitalWrite(TRIGGER_PIN, LOW);
        //# Allow module to settle
        
        //printf("start wait %d\n", GetTimeStamp());
        
        waitforpin(ECHO_PIN, TIMEOUT, 500000L); /* wait 500 ms */
        
        //printf("after wait %d\n", GetTimeStamp());
        
        /* trigger reading */
        digitalWrite(TRIGGER_PIN, HIGH);
        waitforpin(TRIGGER_PIN, TIMEOUT, 10L); /* wait 10 microseconds */
        digitalWrite(TRIGGER_PIN, LOW);
        
        /* wait for reading to start */
        waitforpin(ECHO_PIN, LOW, 5000L); /* 5 ms timeout */
        if (digitalRead(ECHO_PIN) == LOW)
        {
            printf("start wait %d\n", GetTimeStamp());
            pulsewidth = waitforpin(ECHO_PIN, HIGH, 60000L); /* 60 ms timeout */
            printf("after wait %d\n", GetTimeStamp());
            printf("echo at %d micros\n", pulsewidth);
            if (digitalRead(ECHO_PIN) == HIGH)
            {
                /* valid reading code */
                printf("echo at %d micros\n", pulsewidth);
            }
            else
            {
                /* no object detected code */
                printf("echo timed out\n");  
            }  
        }  
        else  
        {  
            /* sensor not firing code */  
            printf("sensor didn't fire\n");  
        }  
    }  
}  



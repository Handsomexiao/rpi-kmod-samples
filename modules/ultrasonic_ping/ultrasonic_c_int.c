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
 
 static volatile unsigned long start = 0;
 static volatile unsigned long now = 0;
 static volatile unsigned int done = 0;
 
  static volatile unsigned int oldState = 0;

//us
 volatile unsigned long GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(unsigned long)1000000+tv.tv_usec;
 }


static volatile int intCount = 0;

/*
 * myInterrupt:
 *********************************************************************************
 */

void myInterrupt (void)
{
    intCount++;
    int newState = digitalRead(ECHO_PIN);

    
    if(newState == HIGH){
            done = 1;
            now =  GetTimeStamp();
    }
    else if(newState == LOW)
    {
        start = GetTimeStamp();
    }
    
}

 int main (int argc, char *argv[])  
 {  

   int pulsewidth;  
   if (wiringPiSetupGpio () == -1)  
   {  
    fprintf (stderr, "Can't initialise wiringPi: %s\n", strerror (errno)) ;  
    return 1 ;  
   }  
   
   pinMode(TRIGGER_PIN, OUTPUT);  
   pinMode(ECHO_PIN, INPUT);  

	if (wiringPiISR (ECHO_PIN, INT_EDGE_BOTH, &myInterrupt) < 0)
    {
        fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
        return 1 ;
    }
    
    int i;
    for (i = 0; i < 100; i++) {
    done = 2;
    
    //# Set trigger to False (Low)
    digitalWrite(TRIGGER_PIN, LOW);  
    //# Allow module to settle	
    delay(500);
    
    //# Send 10us pulse to trigger
    /* trigger reading */  
    digitalWrite(TRIGGER_PIN, HIGH);  
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);  
    
    done = 0;
    
    while(done == 0){
        //printf("waiting : %lu\n", GetTimeStamp());
        delay(100);
        }
        
//         printf("start at %lu \n", start);
//         printf("now at %lu \n", now);


	   pulsewidth = abs(now - start);
       /* valid reading code */  
//        printf("echo at %lu micros\n", pulsewidth);
       
       float timeInSecond = (float)(pulsewidth / 1000000.00F);
       //printf("elaped = %f s\n", timeInSecond);   
       float distance = timeInSecond * 34000.00F;
       distance = distance / 2.00;
       printf("%f\n", distance);   
       
       //printf("intCount = %d \n", intCount);  
    } 
 }  
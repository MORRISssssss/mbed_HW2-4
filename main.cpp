#include "mbed.h"
#include "uLCD_4DGL.h"
#define RISING 0
#define FALLING 1

uLCD_4DGL uLCD(D1, D0, D2);
PwmOut LED(D6);
AnalogIn lightSensor(A0);
AnalogOut  aout(PA_4);
DigitalOut pinA(D8);
InterruptIn pinB(D9);
DigitalOut led1(LED1);

Thread LEDThread; 
Thread sensorThread;
Thread queueThread;
EventQueue queue(32 * EVENTS_EVENT_SIZE);

float sample[100];
int sample_cnt = 0;


void pwmLED()
{
    int cnt = 0;
    while (true)
    {
        LED.pulsewidth_us(5000 * sample[cnt]);
        cnt++;
        if (cnt == 100) cnt = 0;
        ThisThread::sleep_for(10ms);
    }
}

void sensor()
{
    while (true){
        aout = lightSensor.read();
        if (lightSensor.read() > 0.5)
            pinA = 1;
        else
            pinA = 0;
        led1 = pinA;
        sample_cnt++;
        if (sample_cnt > 100000)
            sample_cnt = 0;
        ThisThread::sleep_for(1ms);
    }
}

void printText (int state)
{
    uLCD.locate(3,2);
    if (state == RISING){
        uLCD.color(RED);
        uLCD.printf("RISE");
    }
    else{
        uLCD.color(BLUE);
        uLCD.printf("FALL");
    }
    uLCD.color(WHITE);
    uLCD.locate(3,4);
    uLCD.printf("%5d", sample_cnt);
}

void riseISR ()
{
    queue.call(printText, RISING);
}

void fallISR ()
{
    queue.call(printText, FALLING);
}

int main()
{
    for (int i = 0; i < 100; i++){
        if (i < 25)
            sample[i] = (float)i / 25;
        else if (i < 50)
            sample[i] = 1.;
        else if (i < 75)
            sample[i] = 1. - (i - 50.) / 25;
        else
            sample[i] = 0;
    }
    LED.period_ms(5);
    uLCD.text_width(2);
    uLCD.text_height(2);
    sensorThread.start(sensor);
    LEDThread.start(pwmLED);
    queueThread.start(callback(&queue, &EventQueue::dispatch_forever));
    pinB.rise(&riseISR);
    pinB.fall(&fallISR);
    while (true){
        
        ThisThread::sleep_for(1ms);
    }
}
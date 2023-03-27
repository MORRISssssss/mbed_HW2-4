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

float sample[20];
int sample_cnt = 0;


void pwmLED()
{
    int cnt = 0;
    while (true)
    {
        LED.pulsewidth_us(5000 * sample[cnt]);
        cnt++;
        if (cnt == 20) cnt = 0;
        ThisThread::sleep_for(50ms);
    }
}

void sensor()
{
    float sensorValue[50];
    float averageValue;

    while (true){
        averageValue = 0;
        if (sample_cnt > 5000)
            sample_cnt = 0;
        for (int i = 0; i < 50; i++){
            sensorValue[i] = lightSensor.read();
            sample_cnt++;
            //aout = sensorValue[i];
            averageValue += sensorValue[i];
            ThisThread::sleep_for(1ms);
        }
        averageValue /= 50;
        if (averageValue > 0.4)
            pinA = 1;
        else
            pinA = 0;
        led1 = pinA;
    }
}

void printText (int state, int cnt)
{
    uLCD.locate(2,2);
    if (state == RISING){
        uLCD.color(RED);
        uLCD.printf("RISE");
    }
    else{
        uLCD.color(BLUE);
        uLCD.printf("FALL");
    }
    uLCD.color(WHITE);
    uLCD.locate(2,4);
    uLCD.printf("%4d", cnt);
    printf("%3d\r\n", cnt);
}

void riseISR ()
{
    queue.call(printText, RISING, sample_cnt);
}

void fallISR ()
{
    queue.call(printText, FALLING, sample_cnt);
}

int main()
{
    for (int i = 0; i < 20; i++){
        if (i < 5)
            sample[i] = (float)i / 5;
        else if (i < 10)
            sample[i] = 1.;
        else if (i < 15)
            sample[i] = 1. - (i - 10.) / 5;
        else
            sample[i] = 0;
    }
    LED.period_ms(5);
    uLCD.text_width(2);
    uLCD.text_height(2);
    LEDThread.start(pwmLED);
    sensorThread.start(sensor);
    queueThread.start(callback(&queue, &EventQueue::dispatch_forever));
    pinB.rise(&riseISR);
    pinB.fall(&fallISR);
    while (true){
        aout = pinB;
    }
}
#include "mbed.h"
#include "physcom.h"
#include "rtos.h"
using namespace physcom;

Serial pc(USBTX, USBRX); // open a serial communication to terminal emulator
M3pi robot; //create an object of type  M3pi

Ping Pinger(p11);
AnalogIn passive_light(p16);
DigitalOut leds(p20);
DigitalOut myled(LED1);

#define MAX_SPEED 0.25

#define DARK 0.055
#define STOP_DISTANCE 20

#define DIFFERENCE 0
#define BLACK_LEVEL 450
#define WHITE_LEVEL 50

Thread sideThread;
bool stopped = false;
bool pause = false;

void moveForward()
{
    pc.printf("FRONT \r\n");
    robot.activate_motor(0, MAX_SPEED);
    robot.activate_motor(1, MAX_SPEED);
}

void leftFast()
{
    pc.printf("LEFT FAST \r\n");
    robot.activate_motor(0, MAX_SPEED/2);
    robot.activate_motor(1, MAX_SPEED);
}

void leftSlow()
{
    pc.printf("LEFT \r\n");
    robot.activate_motor(0, -MAX_SPEED);
    robot.activate_motor(1, MAX_SPEED);
}

void rightFast()
{
    pc.printf("RIGHT FAST \r\n");
    robot.activate_motor(0, MAX_SPEED);
    robot.activate_motor(1, MAX_SPEED/2);
}

void rightSlow()
{
    pc.printf("RIGHT \r\n");
    robot.activate_motor(0, MAX_SPEED);
    robot.activate_motor(1, -MAX_SPEED);
}

void stop()
{
    robot.activate_motor(0,0);  // stop left motor
    robot.activate_motor(1,0);  // stop right motor
}

void sosLight()
{
    while(1) {
        for (int i = 0; i < 3; i++) {
            myled = 1;
            wait(0.2);
            myled = 0;
            wait(0.2);
        }
        wait(0.4);
        for (int i = 0; i < 3; i++) {
            myled = 1;
            wait(0.6);
            myled = 0;
            wait(0.2);
        }
        wait(0.4);
        for (int i = 0; i < 3; i++) {
            myled = 1;
            wait(0.2);
            myled = 0;
            wait(0.2);
        }
        wait(1);
    }
}

void lightAndPinger()
{
    float light_value;
    int range;
    while(!stopped) {
        // Automatic lights on detection
        light_value = passive_light.read();
        if (light_value > DARK) {
            pc.printf("Light on");
            leds = 1;
        } else {
            leds = 0;
        }
        pc.printf("value: %f\r\n", light_value);

        // Object detection
        Pinger.Send();
        Thread::wait(40);
        range = Pinger.Read_cm();
        if (range < STOP_DISTANCE) {
            pause = true;
        } else {
            pause = false;
        }
    }
}


int main()
{
    int sensors[5];

    wait(1);
    pc.printf("Start calibration!\r\n");
    robot.sensor_auto_calibrate();   //robot executes calibration
    pc.printf("Finished calibration!\r\n");

    myled = 1;

    pc.printf("Starting in...\r\n");
    for (int i = 5; i > 0; i--) {
        pc.printf("%d\r\n", i);
        wait(1);
    }

    myled = 0;
    wait(0.5);

    sideThread.start(lightAndPinger);

    while (1) {

        if (pause) {
            stop();
        } else {

            robot.calibrated_sensors(sensors);  //5 values are read in a vector

            if ((sensors[0] > BLACK_LEVEL && sensors[1] > BLACK_LEVEL && sensors[2] > BLACK_LEVEL && sensors[3] > BLACK_LEVEL && sensors[4] > BLACK_LEVEL) || (sensors[0] < WHITE_LEVEL && sensors[1] < WHITE_LEVEL && sensors[2] < WHITE_LEVEL && sensors[3] < WHITE_LEVEL && sensors[4] < WHITE_LEVEL)) {

                stop();
                stopped = true;
                sosLight();

            } else {

                if (sensors[0] > BLACK_LEVEL) {
                    leftSlow();
                } else if (sensors[4] > BLACK_LEVEL) {
                    rightSlow();
                } else if (sensors[1] > WHITE_LEVEL && sensors[2] > BLACK_LEVEL && sensors[3] > WHITE_LEVEL ) {
                    moveForward();
                } else if (sensors[1] > sensors[3]) {
                    leftFast();
                } else if (sensors[3] > sensors[1]) {
                    rightFast();
                }

            }

        }

    }
}

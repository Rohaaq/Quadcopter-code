//Rohaan Haq, Diyako G, Adrian C, Logan K
//Quadcopter firmware v2
//This version should have:
// - motor control
// - Bluetooth gamepad control
// - Gyroscopic stabiization (in progress)  

//Libraries
// For controller
#include <Bluepad32.h>
//For servos 
#include <ESP32Servo.h>

//For Gyro
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"


// Note! To use this lib with ESP32
//You need to change the refrence to servo.h in the RC_ESC
// https://esp32.com/viewtopic.php?t=20450#:~:text=It%20has%20a%20buried%20reference%20to%20Servo.h%20which%20must%20be%20changed%20to%20ESP32Servo.h.%20To%20do%20this%2C%20go%20to%20your%20libraries%20folder%20where%20you%20store%20your%20programs%2C%20drill%20down%20to%20RC_ESC%3E%3Esrc%3E%3EESC.h%2C%20and%20edit%20it%20(I%20used%20NotePad%2B%2B).%20Change%20line%2016%20to%20%23include%20%3CESP32Servo.h%3E%20and%20save%20the%20changes
#include <ESC.h>

//Files
#include "config.h"
#include "bluetooth_functions.h"
#include "Stabilization_Gyro.h"

int max_translational = 0;

// Arduino setup function. Runs in CPU 1
void setup() {

    //Runs a bunch of code to configure gyro
    gyro_setup();

    Serial.begin(115200);
    
    // General bluepad info
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "vritual device", is a mouse
    // By default it is disabled.
    BP32.enableVirtualDevice(false);

   
    // The usable range for translational has to be the smaller range
    //The same val is +/- to opposite sides, but we need to avoid a val outside the bounds from being called
    if ( (max_speed - hover_pwm) < hover_pwm - min_speed) {
      max_translational = 0.75*(max_speed - hover_pwm);

    } 
    else {
      max_translational = 0.75*(hover_pwm - min_speed);
    }



    //Arming motors (sends 500 pwm signal - 4th parameter for servo)
    motor_fl.arm();
    motor_fr.arm();
    motor_bl.arm();
    motor_br.arm();   

        
    // This code was taken from some forum to arm motors, since .arm doesn't work for esps
    for (int i=0; i<350; i++){ // run speed from 840 to 1190
      motor_fl.speed(min_speed-200+i); // motor starts up about half way through loop
      motor_fr.speed(min_speed-200+i); // motor starts up about half way through loop
      motor_bl.speed(min_speed-200+i); // motor starts up about half way through loop
      motor_br.speed(min_speed-200+i); // motor starts up about half way through loop
      delay(10);
    }

    delay(5000);  
   
               // Send the Arm value so the ESC will be ready to take commands             
    //Set to true to callibrate ESCs
    //esc_calibrate(false);
    
    while (myControllers[0] && myControllers[0]->isConnected() == false){
      BP32.update();

    }

}

// Arduino loop function. Runs in CPU 1
void loop() {

    // This call fetches all the gamepad info from the NINA (ESP32) module.
    // The gamepads pointer (the ones received in the callbacks) gets updated automatically.
    gyro_offsets();
    BP32.update();

    
    // This guarantees that the gamepad is valid and connected.
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr myController = myControllers[i];

        if (myController && myController->isConnected()) {
            if (myController->isGamepad()) {
                processGamepad(myController);

                //Write speeds to motor

                //CONTROL SCHEME
                // Left stick, up -> Forward. Add to back, subtract from front
                // Left stick, down -> Reverse. Subtract from back, add to front
                // Left stick, left -> Left. Add to right, subtract from left
                // Left stick, up -> Right. Add to left, subtract from right

                // Right stick, up -> Elevate. Add to all.
                // Right stick, down -> Descend. Decrease from all.
                // Right stick, left -> Yaw left. Increase cw pair, decrease ccw pair
                // Right stick, up -> Yaw right. Decrease cw pair, increase ccw pair


                // x -> Forward/back
                // z -> left/right
                // y -> Up/down
                // Yaw -> rotate left/right
                
                //Set of values that will be added/subtracted from each vaules base speed for desired effect

                //Maps joystick range to usable range. + is forward/right/CW
                int x_pwm = map(myController->axisY(), -512, 512, (-1 * max_translational), max_translational);
                int z_pwm = map(myController->axisX(), -512, 512, (-1 * max_translational), max_translational);
                int yaw_pwm = map(myController->axisRX(), -512, 512, (-1 * max_translational), max_translational);
                // Elevation. Gets added to all values
                // Gets mapped to usable range of 
                // This implementation has bugs, there are better ways to do this    
                int y_pwm = map(myController->axisRY(), -512, 512, (-1 * max_translational), max_translational);

                //v2: Only stabilize when static. If not, set offsets to 0
                if (myController->axisY() == 0 && myController->axisX() == 0) {
                    roll_offset = 0;
                    pitch_offset = 0;
                }                


                motor_fl.speed(constrain(((hover_pwm - x_pwm + z_pwm + y_pwm + ( (abs(pitch_offset) + pitch_offset )/2)        + ((abs(roll_offset) + ((-1)*roll_offset) )/2)  ) ), min_speed, max_speed));                                  
                motor_fr.speed(constrain(((hover_pwm - x_pwm - z_pwm + y_pwm + ( (abs(pitch_offset) + pitch_offset )/2)        + ((abs(roll_offset) + pitch_offset )/2)        ) ), min_speed, max_speed));
                motor_bl.speed(constrain(((hover_pwm + x_pwm + z_pwm + y_pwm + ( (abs(pitch_offset) + ((-1)*pitch_offset) )/2) + ((abs(roll_offset) + ((-1)*roll_offset) )/2)  ) ), min_speed, max_speed));
                motor_br.speed(constrain(((hover_pwm + x_pwm - z_pwm + y_pwm + ( (abs(pitch_offset) + ((-1)*pitch_offset) )/2) + ((abs(roll_offset) + pitch_offset )/2)        ) ), min_speed, max_speed));    


            //Sneak this up top?
            } else {
                Serial.printf("Data not available yet\n");
                continue;
            }

        }
    }
    


}

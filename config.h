//General pin configs and constants

//Constants
#define hover_pwm 1460

//Pins for motors
#define motor_pin_fl  19 //Front left  
#define motor_pin_fr  18 //Front right
#define motor_pin_bl  17 //Back left
#define motor_pin_br  16 // Back right

//Minimum and maxiumum speed PWM signals (microseconds) for the ESC
#define max_speed 1860
#define min_speed 1060


// Normalizer function - maps joystick to 

// Servo objects
ESC motor_fl(motor_pin_fl, min_speed, max_speed, 500); 
ESC motor_fr(motor_pin_fr, min_speed, max_speed, 500);
ESC motor_bl(motor_pin_bl, min_speed, max_speed, 500);
ESC motor_br(motor_pin_br, min_speed, max_speed, 500);

/*
// Motors need to be callibrated on occasion. 
// Set this to true, wait for the beeps, power it off, set it to false
//Maybe make a hardware switch for this later?
void esc_calibrate(val) {
  if (val){
    motor_fl.calib();
    motor_fr.calib();
    motor_bl.calib();
    motor_br.calib();
    //Delay 10s for time to power off
    delay(10000);
  }
}
*/
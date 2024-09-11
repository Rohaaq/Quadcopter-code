### Quadcopter Firmware v2 Overview

This code is the second version of a quadcopter firmware designed to control motors, handle Bluetooth gamepad input, and manage gyroscopic stabilization (still in progress). It utilizes libraries such as `Bluepad32` for Bluetooth controller integration, `ESP32Servo` for servo control, and `MPU6050` for gyroscopic data processing.

Key features include:
- **Motor Control**: Configures and controls the four motors of the quadcopter using ESC (Electronic Speed Controllers). The motors are armed and their speed is adjusted based on input from a Bluetooth gamepad.
- **Bluetooth Gamepad Input**: The code allows for real-time control via a Bluetooth gamepad. The controller's joystick movements are mapped to directional and speed adjustments for the quadcopter's motors. Movements include forward, reverse, strafing, elevation, and yaw (rotation).
- **Gyroscopic Stabilization**: While still in development, the code includes a gyroscopic setup for stabilization to maintain balance and adjust motor speeds accordingly when static.

The firmware is structured to receive Bluetooth inputs, calculate motor adjustments, and continuously update motor speeds based on joystick movement and gyroscopic feedback. The gyroscopic offsets help ensure the quadcopter remains stable when no directional input is given.
